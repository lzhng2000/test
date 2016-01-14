#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

const int XXX_TIMER_INTERNVAL = 1;
struct active_object : boost::noncopyable {
    active_object(std::size_t threadPoolSize)
		: m_count(0)
		, m_cntSomeOperation(0)
		, m_cntAOperation(0)
		, m_threadPoolSize(threadPoolSize)
		, m_xxxTimer(m_xxxService, boost::posix_time::seconds(XXX_TIMER_INTERNVAL))
		, m_workxxx(m_xxxService)
#if defined(MULTIPLE_SERVICES)
		, m_workA(m_AService)
#endif
    {
		//startXxxTimer();
		for (std::size_t i = 0; i < m_threadPoolSize; ++i)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&boost::asio::io_service::run, &m_xxxService)));
			m_xxxThreads.push_back(thread);
		}
#if defined(MULTIPLE_SERVICES)
		m_AThread.reset(new boost::thread(
			boost::bind(&boost::asio::io_service::run, &m_AService)));
#endif
	};

    virtual ~active_object() {
#if defined(MULTIPLE_SERVICES)
		m_AService.stop();
		m_AThread->join();
#endif
		m_xxxService.stop();
		// Wait for all threads in the pool to exit.
		for (std::size_t i = 0; i < m_xxxThreads.size(); ++i)
			m_xxxThreads[i]->join();
	};

    void some_operation() {
		std::cout << "Thread: " << boost::this_thread::get_id() << " some_operation()." << std::endl;
		m_xxxService.post(boost::bind(&active_object::some_operation_impl, this));
    };

	void A_operation() {
		std::cout << "Thread: " << boost::this_thread::get_id() << " A_operation()." << std::endl;
#if defined(MULTIPLE_SERVICES)
		m_AService.post(boost::bind(&active_object::A_operation_impl, this));
#else
		m_xxxService.post(boost::bind(&active_object::A_operation_impl, this));
#endif
	};

	void startXxxTimer()
	{
		std::cout << "Thread: " << boost::this_thread::get_id() << " startXxxTimer()." << std::endl;
		m_xxxTimer.cancel();
		m_xxxTimer.async_wait(boost::bind(&active_object::onXxxTimer, this, boost
			::asio::placeholders::error));
	}

	void stopXxxTimer()
	{
		m_xxxTimer.cancel();
	}

protected:

    boost::asio::io_service m_xxxService;
#if defined(MULTIPLE_SERVICES)
	boost::asio::io_service m_AService;
#endif

private:

	int m_count;
	int m_cntSomeOperation;
	int m_cntAOperation;
	std::size_t m_threadPoolSize;
    boost::asio::deadline_timer m_xxxTimer;
    std::vector<boost::shared_ptr<boost::thread> > m_xxxThreads;
	boost::asio::io_service::work m_workxxx;

#if defined(MULTIPLE_SERVICES)
	boost::shared_ptr<boost::thread> m_AThread;
	boost::asio::io_service::work m_workA;
#endif

    void some_operation_impl () {
        // do something here...
        std::cout << "Thread: " << boost::this_thread::get_id() << " some_operation_impl(): " << ++m_cntSomeOperation << std::endl;
    };

	void A_operation_impl() {
		// do something here...
		std::cout << "Thread: " << boost::this_thread::get_id() << " A_operation_impl(): " << ++m_cntAOperation << std::endl;
	};

    void onXxxTimer(boost::system::error_code const & error) {
		std::cout << "Thread: " << boost::this_thread::get_id() << " Timer expires." << std::endl;
		m_xxxTimer.expires_at(m_xxxTimer.expires_at() + boost::posix_time::seconds(XXX_TIMER_INTERNVAL));
		if (!error && ++m_count < 10)
		{
			m_xxxTimer.async_wait(boost::bind(&active_object::onXxxTimer,
				this,
				boost::asio::placeholders::error)
				);
		}
		else
			std::cout << "Thread: " << boost::this_thread::get_id() << " Press any key to exit." << std::endl;

    };

};

int main()
{
	active_object ao (3);
	ao.startXxxTimer();
	for (int i = 0; i< 5; ++i)
	{
		ao.some_operation();
		ao.A_operation();
	}
	getchar();
}