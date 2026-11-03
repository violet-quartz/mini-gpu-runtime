#include "task_queue.h"
#include <iostream>
#include <thread>
#include <string>
#include <stdexcept>
#include <exception>


void worker_loop(TaskQueue& queue) {
    // std::packaged_task 会自动捕获它所执行的任务抛出的异常。wait_and_run() 正常返回 true。
    while (queue.wait_and_run()) {
    }
}

int main() {
    TaskQueue queue;
    std::mutex cout_mutex;

    std::thread worker1([&queue, &cout_mutex] {
        worker_loop(queue);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Worker 1 has exited.\n";
        }
    });

    std::thread worker2([&queue, &cout_mutex] {
        worker_loop(queue);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Worker 2 has exited.\n";
        }
    });

    auto future1 = queue.submit([] {
        return 100 + 200;
    });

    auto future2 = queue.submit([] {
        return std::string("GPU task completed");
    });

    auto future3 = queue.submit([]() -> int {
        throw std::runtime_error("Task failed");
    });

    auto future4 = queue.submit([]() -> int {
        return 400;
    });

    try {
        auto result1 = future1.get();
        std::cout << "Result of task 1: " << result1 << '\n';
    } catch (const std::exception& e) {
        std::cout << "Task 1 failed: " << e.what() << '\n';
    }

    try {
        auto result2 = future2.get();
        std::cout << "Result of task 2: " << result2 << '\n';
    } catch (const std::exception& e) {
        std::cout << "Task 2 failed: " << e.what() << '\n';
    }

    try {
        auto result3 = future3.get();
        std::cout << "Result of task 3: " << result3 << '\n';
    } catch (const std::exception& e) {
        std::cout << "Task 3 failed: " << e.what() << '\n';
    }

    try {
        auto result4 = future4.get();
        std::cout << "Result of task 4: " << result4 << '\n';
    } catch (const std::exception& e) {
        std::cout << "Task 4 failed: " << e.what() << '\n';
    }

    queue.shutdown();

    worker1.join();
    worker2.join();

    return 0;
}