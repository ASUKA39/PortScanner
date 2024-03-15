#include "threadpool.h"

int main() {
    int threadNum = 16;
    ThreadPool pool(threadNum);
    for (int i = 0; i < threadNum; i++) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " is running on core " << sched_getcpu() << std::endl;
        });
    }
    return 0;
}