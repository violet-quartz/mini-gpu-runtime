#include "thread_pool.h"
#include <iostream>
#include <thread>
#include <string>
#include <stdexcept>
#include <exception>
#include <atomic>
#include <cassert>
#include <chrono>

void test_raii() { // 验证 ~ThreadPool() 的正确性， 调用 shutdown()，任务完成后 worker 退出
    std::atomic<int> completed(0);
    {
        ThreadPool pool(2);
        for (int i = 0; i < 10; i++) {
            pool.submit([&completed] {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(10)
                ); // std::thread, 当前线程（正在执行这段 lambda 的线程）
                ++completed;
            });
        }
    }
    // assert(completed.load() == 10); 
    // 如果编译时定义了 NDEBUG，assert() 会被禁用；改为下面的方式，使用 Release 构建，测试也会执行。
    if (completed.load() != 10) {
        throw std::runtime_error("RAII test failed");
    }
}

int main() {
    ThreadPool pool(2);

    auto future1 = pool.submit([] {
        return 100 + 200;
    });

    auto future2 = pool.submit([] {
        return std::string("GPU task completed");
    });

    auto future3 = pool.submit([]() -> int {
        throw std::runtime_error("Task failed");
    });

    auto future4 = pool.submit([]() -> int {
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

    pool.shutdown();
    pool.shutdown();

    try {
        auto future5 = pool.submit([]() -> int {
            return 500;
        });
    } catch (const std::runtime_error& e) {
        std::cout << "Failed to submit task 5. Runtime error: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Failed to submit task 5. Other exception: " << e.what() << "\n";
    }

    try {
        ThreadPool pool2(0);
    } catch (const std::invalid_argument& e) {
        std::cout << "Invalid argument: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Other exception: " << e.what() << "\n";
    }

    test_raii();

    return 0;
}