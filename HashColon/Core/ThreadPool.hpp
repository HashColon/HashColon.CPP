#ifndef HASHCOLON_CORE_THREADPOOL_HPP
#define HASHCOLON_CORE_THREADPOOL_HPP

#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <future>
#include <mutex>
#include <queue>

// This module is inspired by
// * caolan/async: Async JS library : https://github.com/caolan/async
// This module [uses/modified from] the following library
// * vit-vit/CTPL: Cpp Thread Pool Library : https://github.com/vit-vit/CTPL

namespace HashColon::_hidden
{	
	class TSQueue 
	{
	protected:
		using FuncT = std::function<void(int id)>*;
		std::queue<FuncT> q;
		std::mutex m;

	public:
		bool Push(FuncT const& v);
		bool Pop(FuncT& v);
		bool Empty();
	};

	class TSPriorityQueue
	{
	protected:
		using FuncT = std::function<void(int id)>*;
		struct ItemT
		{
			FuncT f;
			size_t priority;
		};
		struct ItemTCmp
		{
			bool operator()(ItemT& a, ItemT& b) { return a.priority < b.priority; };
		};
		
		std::priority_queue<ItemT, std::vector<ItemT>, ItemTCmp> q;
		std::mutex m;

	public:
		bool Push(FuncT const& v, size_t priority);
		bool Pop(FuncT& v);
		bool Empty();		
	};
}

namespace HashColon
{
    class ThreadPool {

    public:

        ThreadPool();
        ThreadPool(int nThreads);

        // the destructor waits for all the functions in the queue to be finished
        ~ThreadPool();

        // get the number of running threads in the pool
        int ThreadCount() const;

        // number of idle threads
        int IdleThreadCount();
        std::thread& GetThread(int i);

        // change the number of threads in the pool
        // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
        // nThreads must be >= 0
        void ChangeThreadCount(int nThreads);

        // empty the queue
        void Clear();

        // pops a functional wrapper to the original function
        std::function<void(int)> Pop();

        // wait for all computing threads to finish and stop all threads
        // may be called asynchronously to not pause the calling thread while waiting
        // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
        void Stop(bool isWait = false);
        void Wait();


        template<typename F, typename... Rest>
        auto Push(F&& f, Rest&&... rest) ->std::future<decltype(f(0, rest...))> {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
                std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
                );
            auto _f = new std::function<void(int id)>([pck](int id) {
                (*pck)(id);
                });
            this->q.Push(_f);
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_one();
            return pck->get_future();
        }

        // run the user's function that excepts argument int - id of the running thread. returned value is templatized
        // operator returns std::future, where the user can get the result and rethrow the catched exceptins
        template<typename F>
        auto Push(F&& f) ->std::future<decltype(f(0))> {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
            auto _f = new std::function<void(int id)>([pck](int id) {
                (*pck)(id);
                });
            this->q.Push(_f);
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_one();
            return pck->get_future();
        }


    private:

        // deleted
        ThreadPool(const ThreadPool&);// = delete;
        ThreadPool(ThreadPool&&);// = delete;
        ThreadPool& operator=(const ThreadPool&);// = delete;
        ThreadPool& operator=(ThreadPool&&);// = delete;

        void SetThread(int i);
        void Init();

        std::vector<std::unique_ptr<std::thread>> threads;
        std::vector<std::shared_ptr<std::atomic<bool>>> flags;
        _hidden::TSQueue q;
        std::atomic<bool> isDone;
        std::atomic<bool> isStop;
        std::atomic<int> nWaiting;  // how many threads are waiting

        std::mutex mutex;
        std::condition_variable cv;
    };

    class PriorityThreadPool 
    {
    public:

        PriorityThreadPool();
        PriorityThreadPool(int nThreads);

        // the destructor waits for all the functions in the queue to be finished
        ~PriorityThreadPool();

        // get the number of running threads in the pool
        int ThreadCount() const;

        // number of idle threads
        int IdleThreadCount();
        std::thread& GetThread(int i);

        // change the number of threads in the pool
        // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
        // nThreads must be >= 0
        void ChangeThreadCount(int nThreads);

        // empty the queue
        void Clear();

        // pops a functional wrapper to the original function
        std::function<void(int)> Pop();

        // wait for all computing threads to finish and stop all threads
        // may be called asynchronously to not pause the calling thread while waiting
        // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
        void Stop(bool isWait = false);
        void Wait();


        template<typename F, typename... Rest>
        auto Push(size_t priority, F&& f, Rest&&... rest) ->std::future<decltype(f(0, rest...))> {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
                std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
                );
            auto _f = new std::function<void(int id)>([pck](int id) {
                (*pck)(id);
                });
            this->q.Push(_f, priority);
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_one();
            return pck->get_future();
        }

        // run the user's function that excepts argument int - id of the running thread. returned value is templatized
        // operator returns std::future, where the user can get the result and rethrow the catched exceptins
        template<typename F>
        auto Push(size_t priority, F&& f) ->std::future<decltype(f(0))> {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
            auto _f = new std::function<void(int id)>([pck](int id) {
                (*pck)(id);
                });
            this->q.Push(_f, priority);
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_one();
            return pck->get_future();
        }

    private:

        // deleted
        PriorityThreadPool(const PriorityThreadPool&);// = delete;
        PriorityThreadPool(PriorityThreadPool&&);// = delete;
        PriorityThreadPool& operator=(const PriorityThreadPool&);// = delete;
        PriorityThreadPool& operator=(PriorityThreadPool&&);// = delete;

        void SetThread(int i);
        void Init();

        std::vector<std::unique_ptr<std::thread>> threads;
        std::vector<std::shared_ptr<std::atomic<bool>>> flags;
        _hidden::TSPriorityQueue q;
        std::atomic<bool> isDone;
        std::atomic<bool> isStop;
        std::atomic<int> nWaiting;  // how many threads are waiting

        std::mutex mutex;
        std::condition_variable cv;
    };
}

#endif
