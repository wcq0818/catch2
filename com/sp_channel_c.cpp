#include <cfloat>

#include <boost/algorithm/string.hpp>

#include <collector/sp_channel_c.h>
#include <collector/decoder_factory.h>
#include <collector/logger.h>
#include <collector/isp_frame.h>
#include <collector/channel_manager.h>
#include <collector/task_scheduler.h>
#include <collector/alarm_event.h>
#include <collector/configure.h>
#include <collector/global.h>
#include <collector/db_connection.h>


sp_session_c::sp_session_c(sp_channel_c* c,
  unsigned int id, unsigned int protocol, sp_arg& spa) noexcept
  :channel_(c)
  , id_(id)
  , type_(0)
  , protocol_(protocol)
  , spa_(spa)
  , io_context_(c->get_io_context())
  , sp_(io_context_)
  , connect_timer_(io_context_)
  , transmit_timer_(io_context_)
  , last_transmit_time_(0)
  , w_op_pos_(0)
  , reconnect_(0)
  , error_(0)
{
  decoder_.reset(decoder_factory::instance()->create_decoder(protocol));
}

sp_session_c::~sp_session_c()
{
}

void sp_session_c::start()
{
  io_context_.post(boost::bind(&sp_session_c::start_callback, shared_from_this()));
}

void sp_session_c::start_callback()
{
  start_connect_timer();
}

int sp_session_c::start_connect()
{
  boost::system::error_code ec;

  sp_.open(spa_.name_, ec);
  if (ec.value() != 0)
    return 1;

  boost::asio::serial_port::baud_rate br(spa_.baud_rate_);
  sp_.set_option(br, ec);
  if (ec.value() != 0)
    return 1;

  boost::asio::serial_port::character_size cs(
    spa_.character_size_);
  sp_.set_option(cs, ec);
  if (ec.value() != 0)
    return 1;

  if (std::abs(spa_.stop_bits_ - 1) < FLT_EPSILON)
  {
    boost::asio::serial_port::stop_bits sb(
      boost::asio::serial_port::stop_bits::one);
    sp_.set_option(sb, ec);
    if (ec.value() != 0)
      return 1;
  }
  else if (std::abs(spa_.stop_bits_ - 2) < FLT_EPSILON)
  {
    boost::asio::serial_port::stop_bits sb(
      boost::asio::serial_port::stop_bits::two);
    sp_.set_option(sb, ec);
    if (ec.value() != 0)
      return 1;
  }
  else if (std::abs(spa_.stop_bits_ - 1.5) < FLT_EPSILON)
  {
    boost::asio::serial_port::stop_bits sb(
      boost::asio::serial_port::stop_bits::onepointfive);
    sp_.set_option(sb, ec);
    if (ec.value() != 0)
      return 1;
  }
  else
    return 1;

  boost::asio::serial_port::parity parity(
    static_cast<boost::asio::serial_port::parity::type>(
      spa_.parity_));
  sp_.set_option(parity, ec);
  if (ec.value() != 0)
    return 1;

  boost::asio::serial_port::flow_control fc(
    static_cast<boost::asio::serial_port::flow_control::type>(
      spa_.flow_control_));
  sp_.set_option(fc, ec);
  if (ec.value() != 0)
    return 1;

  channel_->register_session(shared_from_this());

  last_transmit_time_ = std::time(0);

  start_transmit_timer();
  
  start_read(msg_frag_.space());

  return 0;
}

void sp_session_c::put_msg(boost::shared_ptr<message_block> mb)
{
  io_context_.post(boost::bind(
    &sp_session_c::put_msg_callback, shared_from_this(), mb));
}


void sp_session_c::put_msg_callback(boost::shared_ptr<message_block> mb)
{
  bool write_in_progress = !mq_.empty();

  mq_.push_back(mb);

  if (!write_in_progress)
    start_write();
}


void sp_session_c::start_read(std::size_t bytes_to_read)
{
  sp_.async_read_some(
    boost::asio::buffer(msg_frag_.wr_ptr(), bytes_to_read),
    boost::bind(&sp_session_c::handle_read,
      shared_from_this(), bytes_to_read,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}


void sp_session_c::start_write()
{
  auto mb = mq_.front();
  std::size_t bytes_to_write = mb->length() - w_op_pos_;
  sp_.async_write_some(
    boost::asio::buffer(mb->rd_ptr() + w_op_pos_, bytes_to_write),
    boost::bind(&sp_session_c::handle_write,
      shared_from_this(), bytes_to_write,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}

void sp_session_c::handle_read(
  std::size_t bytes_to_read, const boost::system::error_code& e, std::size_t bytes_transferred)
{
  if (!e && !error_)
  {
    msg_frag_.wr_ptr(bytes_transferred);

    std::vector<boost::shared_ptr<message_block> > out_mb;

    int result = decoder_->decode(msg_frag_, out_mb);

    for (auto& it : out_mb)
    {
      last_transmit_time_ = std::time(0);

      auto frame = boost::make_shared<isp_frame>();

      isp_header header(0,
        0,
        isp_header::et_subscription,
        protocol_,
        0, 
        0, 
        id_, 
        0,
        configure::instance()->get_sys_id(),
        collector_system,
        it->length());

      frame->set_channel_id(id_);
      frame->set_header(header);
      frame->set_body(it);
      channel_->routing_incoming_msg(frame);
    }

    if (result == 0)
      msg_frag_.move();

    start_read(msg_frag_.space());

    return;
  }

  reconnect();
}

void sp_session_c::handle_write(
  std::size_t bytes_to_write,  const boost::system::error_code& e,  std::size_t bytes_transferred)
{
  if (!e && !error_)
  {
    w_op_pos_ += bytes_transferred;

    if (bytes_to_write == bytes_transferred)
    {
      mq_.pop_front();
      w_op_pos_ = 0;
    }

    if (!mq_.empty())
      start_write();

    return;
  }

  reconnect();
}


void sp_session_c::start_connect_timer()
{
  boost::system::error_code ec;
  connect_timer_.expires_after(boost::asio::chrono::seconds(2));
  connect_timer_.async_wait(
    boost::bind(
      &sp_session_c::handle_connect_timeout,
      shared_from_this(),
      boost::asio::placeholders::error));
}

void sp_session_c::stop_connect_timer()
{
  boost::system::error_code ec;
  connect_timer_.cancel(ec);
}

void sp_session_c::handle_connect_timeout(const boost::system::error_code& e)
{
  if (start_connect() != 0)
    reconnect();
}

void sp_session_c::start_transmit_timer()
{
  boost::system::error_code ec;
  transmit_timer_.expires_after(boost::asio::chrono::seconds(20));
  transmit_timer_.async_wait(
    boost::bind(&sp_session_c::handle_transmit_timeout,
      shared_from_this(),
      boost::asio::placeholders::error));
}

void sp_session_c::stop_transmit_timer()
{
  boost::system::error_code ec;
  transmit_timer_.cancel(ec);
}

void sp_session_c::handle_transmit_timeout(const boost::system::error_code& e)
{
  if (!e && !error_)
  {
    auto now = std::time(0);
    if (now - last_transmit_time_ < 60)
    {
      start_transmit_timer();
      return;
    }
  }

  reconnect();
}

void sp_session_c::reconnect()
{
  error_ = 1;

  if (!reconnect_)
  {
    boost::system::error_code ec;
    sp_.close(ec);
    channel_->remove_session(shared_from_this());
    
    channel_->reconnect(id_, protocol_, spa_);

    reconnect_ = 1;
  }
}


sp_channel_c::sp_channel_c(channel_manager* cm) noexcept
  :channel_base(3, cm)
{
}

sp_channel_c::~sp_channel_c()
{
}

void sp_channel_c::reconnect(unsigned int id, unsigned int protocol, sp_arg& spa)
{
  auto s = boost::make_shared<sp_session_c>(this, id, protocol, spa);
  s->start();
}

int sp_channel_c::start()
{
  char sql[1024] = { 0 };
  sprintf(sql, "select id, protocol_type, name, baud_rate, "
    "flow_control, parity, stop_bits, character_size from sp_channel where use_flag = 1 ");

  db_connection dbc(configure::instance()->get_db_arg());
  
  dbc.logon();

  int error = 0;
  int sum = 0;
  try
  {
    otl_stream s;
    s.open(32, sql, dbc.get_db_connection());

    while (!s.eof())
    {
      unsigned int id = 0;
      unsigned int protocol = 0;

      sp_arg spa;

      s >> id;
      s >> protocol;
      s >> spa.name_;
      s >> spa.baud_rate_;
      s >> spa.flow_control_;
      s >> spa.parity_;
      s >> spa.stop_bits_;
      s >> spa.character_size_;

      boost::algorithm::trim(spa.name_);

      auto s = boost::make_shared<sp_session_c>(this, id, protocol, spa);
      s->start();

      sum++;
    }
  }
  catch (otl_exception& e)
  {
    logger::instance()->log(SL_ERROR, "%d, %s, %s, %s, %s",
      e.code, e.msg, e.sqlstate, e.stm_text, e.var_info);
    error = -1;
  }

  dbc.logoff();

  return error == 0 ? error : sum;
}

int sp_channel_c::routing_outgoing_msg(
  unsigned int id, unsigned int type, boost::shared_ptr<message_block> mb)
{
  modbus_sp_session_ptr s;
  if (find_session(id, s) == 0)
  {
    s->put_msg(mb);
    return 0;
  }

  return -1;
}

void sp_channel_c::register_session(modbus_sp_session_ptr s)
{
  boost::recursive_mutex::scoped_lock l(session_map_mutex_);

  session_map_[s->get_id()] = s;
  cm_->bind_session_to_channel(s->get_id(), id_);
}


void sp_channel_c::remove_session(modbus_sp_session_ptr s)
{
  boost::recursive_mutex::scoped_lock l(session_map_mutex_);

  auto found = false;

  auto it = session_map_.find(s->get_id());
  if (it != session_map_.end())
  {
    if (s == it->second)
    {
      session_map_.erase(it);
      found = true;
    }
  }

  if (found)
    cm_->unbind_session_from_channel(s->get_id());
}


int sp_channel_c::find_session(unsigned int id, modbus_sp_session_ptr& s)
{
  boost::recursive_mutex::scoped_lock l(session_map_mutex_);

  auto it = session_map_.find(id);
  if (it != session_map_.end())
  {
    s = it->second;
    return 0;
  }

  return -1;
}

