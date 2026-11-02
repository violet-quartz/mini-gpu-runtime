#include "task_queue.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <future>

void worker_loop(TaskQueue& queue) {
    while (queue.wait_and_run()) {
    }
}

int main() {
    TaskQueue queue;

    std::thread worker1([&queue] {
        worker_loop(queue);
    });

    std::thread worker2([&queue] {
        worker_loop(queue);
    });

    std::atomic<int> completed{0};
    std::mutex cout_mutex;

    for (int i = 0; i < 5; i++) {
        queue.submit([i, &completed, &cout_mutex](){
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Task " << i << " is running\n";
            }
            ++completed;
        });
    }

    queue.shutdown();

    worker1.join();
    worker2.join();

    std::cout << "Completed: " << completed.load() << '\n';

    return 0;
}