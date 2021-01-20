
#ifndef COLLECTOR_SP_CHANNEL_H
#define COLLECTOR_SP_CHANNEL_H


#include <deque>

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/enable_shared_from_this.hpp>


#include <collector/decoder.h>
#include <collector/sp_arg.h>
#include <collector/message_block.h>
#include <collector/channel_base.h>


class channel_manager;

class sp_channel_c;

class sp_session_c : public boost::enable_shared_from_this<sp_session_c>
{
public: 
  
  explicit sp_session_c(sp_channel_c* c,
    unsigned int id, unsigned int protocol, sp_arg& spa) noexcept;

  virtual ~sp_session_c();

  unsigned int get_id() const { return id_; }

  unsigned int get_type() const { return type_; }

  unsigned int get_prtocol() const { return protocol_; }
    
  void start();

  void put_msg(boost::shared_ptr<message_block> mb);  

  void reconnect();

private:

  void start_callback();

  int start_connect();

  void start_connect_timer();

  void stop_connect_timer();

  void start_transmit_timer();

  void stop_transmit_timer();

  void handle_transmit_timeout(const boost::system::error_code& e);
  
  void handle_connect_timeout(const boost::system::error_code& e);

  void put_msg_callback(boost::shared_ptr<message_block> mb);

  void start_read(std::size_t bytes_to_read);

  void start_write();

  void handle_read(std::size_t bytes_to_read,  
    const boost::system::error_code& e,  std::size_t bytes_transferred);

  void handle_write(std::size_t bytes_to_write,
    const boost::system::error_code& e, std::size_t bytes_transferred);

  unsigned int id_;

  unsigned int type_;

  unsigned int protocol_;

  boost::asio::io_context& io_context_;

  sp_channel_c* channel_;

  std::deque<boost::shared_ptr<message_block> > mq_;

  decoder_ptr decoder_;

  sp_arg spa_;

  boost::asio::serial_port sp_;

  message_block msg_frag_;

  volatile std::size_t w_op_pos_;

  volatile int error_;

  volatile int reconnect_;

  boost::asio::steady_timer connect_timer_;

  std::time_t last_transmit_time_;

  boost::asio::steady_timer transmit_timer_;
};

typedef boost::shared_ptr<sp_session_c> modbus_sp_session_ptr;



class sp_channel_c : public channel_base
{
public:

  explicit sp_channel_c(channel_manager* ca) noexcept;

  virtual ~sp_channel_c();

  virtual int routing_outgoing_msg(
    unsigned int id, unsigned int type, boost::shared_ptr<message_block> mb) override;
  
  virtual int start() override;

  void register_session(modbus_sp_session_ptr c);

  void remove_session(modbus_sp_session_ptr c);

  int find_session(unsigned int id, modbus_sp_session_ptr& c);

  void reconnect(unsigned int id, unsigned int protocol, sp_arg& spa);

private:

  using session_map = boost::unordered_map<unsigned int, modbus_sp_session_ptr>;
  
  session_map session_map_;

  boost::recursive_mutex session_map_mutex_;
 
};


#endif