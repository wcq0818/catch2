#pragma once
#include <queue>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <condition_variable>

class thread_task
{
public:
	thread_task(){};
	virtual ~thread_task(){};
	virtual void execute()
	{
// 		printf("execute...\n");
// 		boost::this_thread::sleep(boost::posix_time::seconds(30));
	}
};

typedef boost::shared_ptr<thread_task> thread_task_ptr;

class thread_task_queue
{
public:
	void push_task(const thread_task_ptr task)
	{
 		//boost::mutex::scoped_lock lock(task_queue_mutex_);
		//boost::unique_lock<boost::mutex> lock(task_queue_mutex_);
		printf("push_task begin...ID = %d\n", boost::this_thread::get_id());
		//boost::this_thread::sleep(boost::posix_time::seconds(20));
		task_queue_.push(task);
		//lock.unlock();
		printf("unlock begin...ID = %d\n", boost::this_thread::get_id());
		boost::this_thread::sleep(boost::posix_time::seconds(5));
		printf("notify_one begin...ID = %d\n", boost::this_thread::get_id());
		cond_.notify_one();
		
		//cond_.notify_all();	
		
		//lock.unlock();
		boost::this_thread::sleep(boost::posix_time::seconds(5));
		printf("push_task end...  ID = %d\n", boost::this_thread::get_id());
	}
	thread_task_ptr get_task()
	{
		//boost::mutex::scoped_lock enter_lock(enter_mutex_);
		printf("uni_lock begin... ID = %d\n", boost::this_thread::get_id());
		boost::unique_lock<boost::mutex> lock(task_queue_mutex_);
		printf("get_task begin... ID = %d\n", boost::this_thread::get_id());
		//boost::mutex::scoped_lock lock(task_queue_mutex_);
		while (task_queue_.empty())
		{
			printf("wait begin...     ID = %d\n", boost::this_thread::get_id());
			cond_.wait(lock);
// 			if (cond_.wait_for(lock, boost::chrono::milliseconds(5000)) == boost::cv_status::timeout)
// 			{
// 				printf("wait end time_out ...     ID = %d\n", boost::this_thread::get_id());
// 			}
// 			else
// 			{
// 				printf("wait end notified ...     ID = %d\n", boost::this_thread::get_id());
// 			}
		}
		thread_task_ptr task(task_queue_.front());
		task_queue_.pop();
		printf("get_task end...   ID = %d\n", boost::this_thread::get_id());
		boost::this_thread::sleep(boost::posix_time::seconds(10));
		return task;
	}
	bool empty() const
    {
        boost::mutex::scoped_lock lock(task_queue_mutex_);
        return task_queue_.empty();
    }
private:
	std::queue<thread_task_ptr> task_queue_;
	mutable boost::mutex task_queue_mutex_;
	boost::condition_variable_any cond_;
};

class thread_pool
{
public:
	thread_pool(int num):thread_num(num),is_run(false){}
	~thread_pool(){}
	void start()
	{
		is_run=true;
		if(thread_num<=0)
			return;
		for(int i=0;i<thread_num;++i)
		{
			thread_group_.add_thread(new boost::thread(boost::bind(&thread_pool::run,this)));
		}	
	}

	void stop()
	{
		is_run=false;
	}

	void put(const thread_task_ptr task)
	{
		task_queue_.push_task(task);
	}

	void join()
	{
		thread_group_.join_all();
	}

private:
	thread_task_queue task_queue_;
	boost::thread_group thread_group_;
	int thread_num;
	volatile bool is_run;
	void run()
	{
		while(is_run)
		{
			thread_task_ptr task=task_queue_.get_task();
			task->execute();
		}
	}
};