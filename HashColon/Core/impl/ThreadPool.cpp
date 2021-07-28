#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <future>
#include <mutex>
#include <queue>

#include <HashColon/Core/ThreadPool.hpp>

using namespace std;

namespace HashColon::_hidden
{
	bool TSQueue::Push(FuncT const& v)
	{
        unique_lock<mutex> lock(this->m);
        this->q.push(v);
        return true;
	}

    // deletes the retrieved element, do not use for non integral types
    bool TSQueue::Pop(FuncT& v) {
        unique_lock<mutex> lock(this->m);
        if (this->q.empty())
            return false;
        v = this->q.front();
        this->q.pop();
        return true;
    }

    bool TSQueue::Empty() {
        unique_lock<mutex> lock(this->m);
        return this->q.empty();
    }

    bool TSPriorityQueue::Push(FuncT const& v, size_t priority)
    {
        unique_lock<mutex> lock(this->m);
        this->q.push(ItemT{ v, priority });
        return true;
    }

    // deletes the retrieved element, do not use for non integral types
    bool TSPriorityQueue::Pop(FuncT& v) {
        unique_lock<mutex> lock(this->m);
        if (this->q.empty())
            return false;
        v = this->q.top().f;
        this->q.pop();
        return true;
    }

    bool TSPriorityQueue::Empty() {
        unique_lock<mutex> lock(this->m);
        return this->q.empty();
    }
}

namespace HashColon
{
    ThreadPool::ThreadPool() { this->Init(); }
    ThreadPool::ThreadPool(int nThreads) { this->Init(); this->ChangeThreadCount(nThreads); }

    // the destructor waits for all the functions in the queue to be finished
    ThreadPool::~ThreadPool() {
        this->Stop(true);
    }

    // get the number of running threads in the pool
    int ThreadPool::ThreadCount() const { return static_cast<int>(this->threads.size()); }

    // number of idle threads
    int ThreadPool::IdleThreadCount() { return this->nWaiting; }
    thread& ThreadPool::GetThread(int i) { return *this->threads[i]; }

    // change the number of threads in the pool
    // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
    // nThreads must be >= 0
    void ThreadPool::ChangeThreadCount(int nThreads) {
        if (!this->isStop && !this->isDone) {
            int oldNThreads = static_cast<int>(this->threads.size());
            if (oldNThreads <= nThreads) {  // if the number of threads is increased
                this->threads.resize(nThreads);
                this->flags.resize(nThreads);

                for (int i = oldNThreads; i < nThreads; ++i) {
                    this->flags[i] = std::make_shared<std::atomic<bool>>(false);
                    this->SetThread(i);
                }
            }
            else {  // the number of threads is decreased
                for (int i = oldNThreads - 1; i >= nThreads; --i) {
                    *this->flags[i] = true;  // this thread will finish
                    this->threads[i]->detach();
                }
                {
                    // stop the detached threads that were waiting
                    std::unique_lock<std::mutex> lock(this->mutex);
                    this->cv.notify_all();
                }
                this->threads.resize(nThreads);  // safe to delete because the threads are detached
                this->flags.resize(nThreads);  // safe to delete because the threads have copies of shared_ptr of the flags, not originals
            }
        }
    }

    // empty the queue
    void ThreadPool::Clear() {
        std::function<void(int id)>* _f;
        while (this->q.Pop(_f))
            delete _f; // empty the queue
    }

    // pops a functional wrapper to the original function
    std::function<void(int)> ThreadPool::Pop() {
        std::function<void(int id)>* _f = nullptr;
        this->q.Pop(_f);
        std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
        std::function<void(int)> f;
        if (_f)
            f = *_f;
        return f;
    }

    // wait for all computing threads to finish and stop all threads
    // may be called asynchronously to not pause the calling thread while waiting
    // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
    void ThreadPool::Stop(bool isWait) {
        if (!isWait) {
            if (this->isStop)
                return;
            this->isStop = true;
            for (int i = 0, n = this->ThreadCount(); i < n; ++i) {
                *this->flags[i] = true;  // command the threads to stop
            }
            this->Clear();  // empty the queue
        }
        else {
            if (this->isDone || this->isStop)
                return;
            this->isDone = true;  // give the waiting threads a command to finish
        }
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_all();  // stop all waiting threads
        }
        for (int i = 0; i < static_cast<int>(this->threads.size()); ++i) {  // wait for the computing threads to finish
            if (this->threads[i]->joinable())
                this->threads[i]->join();
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
        // therefore delete them here
        this->Clear();
        this->threads.clear();
        this->flags.clear();
    }

    void ThreadPool::Wait()
    {
        this->Stop(true);
    }

    void ThreadPool::SetThread(int i) 
    {
        std::shared_ptr<std::atomic<bool>> flag(this->flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag/* a copy of the shared ptr to the flag */]() {
            std::atomic<bool>& _flag = *flag;
            std::function<void(int id)>* _f;
            bool isPop = this->q.Pop(_f);
            while (true) {
                while (isPop) {  // if there is anything in the queue
                    std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
                    (*_f)(i);
                    if (_flag)
                        return;  // the thread is wanted to stop, return even if the queue is not empty yet
                    else
                        isPop = this->q.Pop(_f);
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(this->mutex);
                ++this->nWaiting;
                this->cv.wait(lock, [this, &_f, &isPop, &_flag]() { isPop = this->q.Pop(_f); return isPop || this->isDone || _flag; });
                --this->nWaiting;
                if (!isPop)
                    return;  // if the queue is empty and this->isDone == true or *flag then return
            }
        };
        this->threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
    }

    void ThreadPool::Init() { this->nWaiting = 0; this->isStop = false; this->isDone = false; }
  
}

namespace HashColon
{
    PriorityThreadPool::PriorityThreadPool() { this->Init(); }
    PriorityThreadPool::PriorityThreadPool(int nThreads) { this->Init(); this->ChangeThreadCount(nThreads); }

    // the destructor waits for all the functions in the queue to be finished
    PriorityThreadPool::~PriorityThreadPool() {
        this->Stop(true);
    }

    // get the number of running threads in the pool
    int PriorityThreadPool::ThreadCount() const { return static_cast<int>(this->threads.size()); }

    // number of idle threads
    int PriorityThreadPool::IdleThreadCount() { return this->nWaiting; }
    thread& PriorityThreadPool::GetThread(int i) { return *this->threads[i]; }

    // change the number of threads in the pool
    // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
    // nThreads must be >= 0
    void PriorityThreadPool::ChangeThreadCount(int nThreads) {
        if (!this->isStop && !this->isDone) {
            int oldNThreads = static_cast<int>(this->threads.size());
            if (oldNThreads <= nThreads) {  // if the number of threads is increased
                this->threads.resize(nThreads);
                this->flags.resize(nThreads);

                for (int i = oldNThreads; i < nThreads; ++i) {
                    this->flags[i] = std::make_shared<std::atomic<bool>>(false);
                    this->SetThread(i);
                }
            }
            else {  // the number of threads is decreased
                for (int i = oldNThreads - 1; i >= nThreads; --i) {
                    *this->flags[i] = true;  // this thread will finish
                    this->threads[i]->detach();
                }
                {
                    // stop the detached threads that were waiting
                    std::unique_lock<std::mutex> lock(this->mutex);
                    this->cv.notify_all();
                }
                this->threads.resize(nThreads);  // safe to delete because the threads are detached
                this->flags.resize(nThreads);  // safe to delete because the threads have copies of shared_ptr of the flags, not originals
            }
        }
    }

    // empty the queue
    void PriorityThreadPool::Clear() {
        std::function<void(int id)>* _f;
        while (this->q.Pop(_f))
            delete _f; // empty the queue
    }

    // pops a functional wrapper to the original function
    std::function<void(int)> PriorityThreadPool::Pop() {
        std::function<void(int id)>* _f = nullptr;
        this->q.Pop(_f);
        std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
        std::function<void(int)> f;
        if (_f)
            f = *_f;
        return f;
    }

    // wait for all computing threads to finish and stop all threads
    // may be called asynchronously to not pause the calling thread while waiting
    // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
    void PriorityThreadPool::Stop(bool isWait) {
        if (!isWait) {
            if (this->isStop)
                return;
            this->isStop = true;
            for (int i = 0, n = this->ThreadCount(); i < n; ++i) {
                *this->flags[i] = true;  // command the threads to stop
            }
            this->Clear();  // empty the queue
        }
        else {
            if (this->isDone || this->isStop)
                return;
            this->isDone = true;  // give the waiting threads a command to finish
        }
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_all();  // stop all waiting threads
        }
        for (int i = 0; i < static_cast<int>(this->threads.size()); ++i) {  // wait for the computing threads to finish
            if (this->threads[i]->joinable())
                this->threads[i]->join();
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
        // therefore delete them here
        this->Clear();
        this->threads.clear();
        this->flags.clear();
    }

    void PriorityThreadPool::Wait()
    {
        this->Stop(true);
    }

    void PriorityThreadPool::SetThread(int i)
    {
        std::shared_ptr<std::atomic<bool>> flag(this->flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag/* a copy of the shared ptr to the flag */]() {
            std::atomic<bool>& _flag = *flag;
            std::function<void(int id)>* _f;
            bool isPop = this->q.Pop(_f);
            while (true) {
                while (isPop) {  // if there is anything in the queue
                    std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
                    (*_f)(i);
                    if (_flag)
                        return;  // the thread is wanted to stop, return even if the queue is not empty yet
                    else
                        isPop = this->q.Pop(_f);
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(this->mutex);
                ++this->nWaiting;
                this->cv.wait(lock, [this, &_f, &isPop, &_flag]() { isPop = this->q.Pop(_f); return isPop || this->isDone || _flag; });
                --this->nWaiting;
                if (!isPop)
                    return;  // if the queue is empty and this->isDone == true or *flag then return
            }
        };
        this->threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
    }

    void PriorityThreadPool::Init() { this->nWaiting = 0; this->isStop = false; this->isDone = false; }

}