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
        workers.emplace_back([this, cpuset] {   // lambda function to create task for each thread
            pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset); // set thread affinity

            while(true) {   // main loop of thread
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);    // lock the critical section
                    this->condition.wait(lock, [this] {  // wait for the condition to be true
                        return this->stop || !this->tasks.empty();  // condition is stop or tasks is not empty
                    });
                    if (this->stop && this->tasks.empty()) {    // if stop is true and tasks is empty
                        return;
                    }
                    task = std::move(this->tasks.front());  // get the task from the front of the queue
                    this->tasks.pop();  // remove the task from the queue
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