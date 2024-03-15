#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <coroutine>
#include <type_traits>

#include "threadpool.h"

using namespace std;

constexpr int MAX_THREADS = 16;
constexpr int MAX_COROUTINES_PER_THREAD = 16;

ThreadPool::ThreadPool() {
    for (int i = 0; i < MAX_THREADS; ++i) {
        threads.emplace_back(std::bind(&ThreadPool::threadFunction, this, i));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : threads) {
        worker.join();
    }
}

template<typename Func, typename... Args>
auto ThreadPool::enqueue(Func &&func, Args &&... args) {
    using ReturnType = std::invoke_result_t<Func, Args...>;

    std::shared_ptr<std::packaged_task<ReturnType()>> task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );

    std::future<ReturnType> future = task->get_future();

    {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.emplace([task]() { (*task)(); });
    }

    condition.notify_one();

    return future;
}

void ThreadPool::threadFunction(int cpu_core) {
    // Bind thread to CPU core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_core, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    // Create coroutine pool
    for (int i = 0; i < MAX_COROUTINES_PER_THREAD; ++i) {
        coroutineFunction();
    }

    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this]() { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

void ThreadPool::coroutineFunction() {
    std::cout << "Coroutine running on thread " << std::this_thread::get_id() << std::endl;
    // Placeholder coroutine logic
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
