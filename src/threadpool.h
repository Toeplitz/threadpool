#pragma once
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/*
 Implementastions based on:

 @book{williams2012c++,
  title={C++ concurrency in action},
  author={Williams, Anthony},
  year={2012},
  publisher={London}
}

(Williams, 2012) Page: 154

*/

template <typename T>
class threadsafe_queue
{
  private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

  public:
    threadsafe_queue()
    {
    }

    void push(T data)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(data));
        data_cond.notify_one();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });

        std::shared_ptr<T> res(
            std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop(T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        if (data_queue.empty())
        {
            return false;
        }
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
};

/*
(Williams, 2012) Page: 248
*/
class join_threads
{
    std::vector<std::thread> &threads;

  public:
    explicit join_threads(std::vector<std::thread> &threads_) : threads(threads_)
    {
    }

    ~join_threads()
    {
        for (unsigned long i = 0; i < threads.size(); ++i)
        {
            if (threads[i].joinable())
            {
                threads[i].join();
            }
        }
    }
};

/*
(Williams, 2012) Page: 278
*/
class function_wrapper
{
    struct impl_base
    {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;
    template <typename F>
    struct impl_type : impl_base
    {
        F f;
        impl_type(F &&f_) : f(std::move(f_)) {}
        void call() { f(); }
    };

  public:
    template <typename F>
    function_wrapper(F &&f) : impl(new impl_type<F>(std::move(f)))
    {
    }

    void operator()() { impl->call(); }

    function_wrapper() = default;

    function_wrapper(function_wrapper &&other) : impl(std::move(other.impl))
    {
    }

    function_wrapper &operator=(function_wrapper &&other)
    {
        impl = std::move(other.impl);
        return *this;
    }

    function_wrapper(const function_wrapper &) = delete;
    function_wrapper(function_wrapper &) = delete;
    function_wrapper &operator=(const function_wrapper &) = delete;
};

class ThreadPool
{
    std::atomic_bool done;
    threadsafe_queue<function_wrapper> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;

    void worker_thread()
    {
        while (!done)
        {
            function_wrapper task;
            if (work_queue.try_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    };

  public:
    ThreadPool() : done(false), joiner(threads)
    {
        unsigned const thread_count = std::thread::hardware_concurrency();
        /* unsigned const thread_count = 1; */

        std::cout << "Creating " << thread_count << " threads in a pool" << std::endl;
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                threads.push_back(std::thread(&ThreadPool::worker_thread, this));
            }
        }
        catch (...)
        {
            done = true;
            throw;
        }
    }

    ~ThreadPool()
    {
        done = true;
    }

    template <typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;

        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push(std::move(task));

        return res;
    }
};