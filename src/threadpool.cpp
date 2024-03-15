#include <functional>
#include <pthread.h>
#include <sched.h>

#include "threadpool.h"

ThreadPool::ThreadPool(int numThreads) : stop(false) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    int numCore = std::thread::hardware_concurrency();
    for (int i = 0; i < numThreads; i++) {
        CPU_SET(i % numCore, &cpuset);
        workers.emplace_back([this, cpuset] {
            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

            while(true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    if (this->stop && this->tasks.empty()) {
                        return;
                    }
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(task);
    }
    condition.notify_one();
}