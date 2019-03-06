#include "catch.hpp"
#include "kms\service_pool.h"
#include "logger\logger.h"
#include <boost\thread.hpp>
#include <boost\asio.hpp>
#include "tools\common_func.h"

///////////////////////////////////////////////////////////////////////////////////////////
class BaseChannel : public std::enable_shared_from_this<BaseChannel>
{
public:
	using IOService = boost::asio::io_service;
	using ThreadGroup = boost::thread_group;

	BaseChannel(unsigned int ioThreadCount = 2);
	virtual ~BaseChannel();

	virtual void start();
	virtual void join();
	virtual void stop();

public:
	unsigned int getIOThreadCount() const { return ioThreadCount_; }
	IOService & getIOService() { return ioService_; }

public:
	ThreadGroup threadPool_;
	IOService ioService_;
	unsigned int ioThreadCount_;
};

BaseChannel::BaseChannel(unsigned int ioThreadCount)
	: ioThreadCount_(ioThreadCount)
{
}

BaseChannel::~BaseChannel()
{
}

void BaseChannel::start()
{
	for (unsigned int i = 0; i < ioThreadCount_; i++)
	{
		threadPool_.create_thread(boost::bind(&boost::asio::io_service::run, &ioService_));
	}
}

void BaseChannel::join()
{
	threadPool_.join_all();
}

void BaseChannel::stop()
{
	ioService_.stop();
}

///////////////////////////////////////////////////////////////////////////////////////////
#define POST_NUM 2
class BaseChannelHandler : public std::enable_shared_from_this<BaseChannelHandler>
{
public:
	using Ptr = std::shared_ptr<BaseChannelHandler>;

public:
	long long sub = 0;
	long long sub2 = 0;
	int post_value_[POST_NUM] = {0};
	boost::asio::io_context::strand strand_;
	boost::asio::deadline_timer loginTimer_;
	boost::asio::deadline_timer ConnectTimer_;
	const int tick_ = 20;
public:
	static bool callback_fun(int meter_id, kms_response* rsp);

public:
	BaseChannelHandler(BaseChannel& channel)
		: strand_(channel.getIOService())
		, loginTimer_(channel.getIOService())
		, ConnectTimer_(channel.getIOService())
	{
	}
	void login(const boost::system::error_code& error)
	{
		while (sub < tick_)
		{
			sleep(1);
			sub++;
			printf("login sub = %d, thread = %d\n", sub, boost::this_thread::get_id());
		}
	}
	void connect(const boost::system::error_code& error)
	{
		while (sub2 < tick_)
		{
			sleep(1);
			sub2++;
			printf("connect sub2 = %d, thread = %d\n", sub2, boost::this_thread::get_id());
		}
	}
	void post(int& id, int& post_value)
	{
		while (post_value < tick_)
		{
			sleep(1);
			post_value++;
			printf("post id = %d, value = %d, thread = %d\n", id, post_value, boost::this_thread::get_id());
		}
	}

	void start_time()
	{
		loginTimer_.expires_from_now(boost::posix_time::seconds(0));
		loginTimer_.async_wait(strand_.wrap(boost::bind(&BaseChannelHandler::login, BaseChannelHandler::shared_from_this(), boost::asio::placeholders::error)));

		ConnectTimer_.expires_from_now(boost::posix_time::seconds(1));
		ConnectTimer_.async_wait(strand_.wrap(boost::bind(&BaseChannelHandler::connect, BaseChannelHandler::shared_from_this(), boost::asio::placeholders::error)));

	}
	void start_post()
	{
		for (int id = 0; id < POST_NUM; id++)
		{
			strand_.post((boost::bind(&BaseChannelHandler::post, BaseChannelHandler::shared_from_this(), id, post_value_[id])));
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("ioservice", "[ioservice][.hide]")
{
	BaseChannel channel(POST_NUM);
	BaseChannelHandler::Ptr handler;
	handler.reset(new BaseChannelHandler(channel));
	handler->start_time();
	channel.start();

	BaseChannel channel1(POST_NUM);
	BaseChannelHandler::Ptr handler1;
	handler1.reset(new BaseChannelHandler(channel1));
	handler1->start_post();
	channel1.start();

	channel.join();
	channel1.join();
}

