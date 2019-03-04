#include <iostream>
#include <threadpool.h>

#include <chrono>

int main()
{
    ThreadPool tp;
    uint64_t N = 100;

    std::atomic_int ongoing_count(0);
    std::atomic_int completed_count(0);

    for (uint64_t n(0); n < N; ++n)
    {
        // Delay submitting to get clean looking output...
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        tp.submit([n, &ongoing_count, &completed_count]() {
            ongoing_count++;

            const uint32_t sleep_time = 1000;
            std::thread::id this_id = std::this_thread::get_id();
            std::cout << "[ " << n << "] sleeping for " << sleep_time << " in thread " << this_id << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            std::cout << "[ " << n << "] exiting ..." << std::endl;

            completed_count++;
            ongoing_count--;
        });
    }

    while (1)
    {
        std::cout << "ongoing: " << ongoing_count << ", completed: " << completed_count
                  << " / " << N << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (completed_count == N)
        {
            std::cout << "Finished: " << completed_count << std::endl;
            break;
        }
    }

    return 0;
}