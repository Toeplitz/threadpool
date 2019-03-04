#include <iostream>
#include <threadpool.h>

#include <chrono>

class Foo
{

  public:
    void do_work(const uint64_t &n)
    {
        const uint32_t sleep_time = 1000;
        std::thread::id this_id = std::this_thread::get_id();
        // std::cout << "[ " << n << "] sleeping for " << sleep_time << " in thread " << this_id << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        // std::cout << "[ " << n << "] exiting ..." << std::endl;
    }
};

int main()
{
    ThreadPool tp;
    threadsafe_queue<std::unique_ptr<Foo>> queue_in;
    threadsafe_queue<std::unique_ptr<Foo>> queue_out;

    uint64_t N = 100;

    std::atomic_int ongoing_count(0);
    std::atomic_int completed_count(0);

    for (uint64_t n(0); n < N; ++n)
    {
        std::unique_ptr<Foo> foo(new Foo());
        queue_in.push(std::move(foo));
    }

    for (uint64_t n(0); n < N; ++n)
    {
        tp.submit([n, &queue_in, &queue_out]() {
            std::unique_ptr<Foo> foo;

            if (queue_in.try_pop(foo))
            {
                foo->do_work(n);
                queue_out.push(std::move(foo));
            }
        });
    }

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "queue_in: " << queue_in.size() << ", queue_out: " << queue_out.size() << std::endl;

        if (queue_out.size() == N)
        {
            std::cout << "Exiting main process ... " << std::endl;
            break;
        }
    }

    return 0;
}