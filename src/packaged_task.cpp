#include <iostream>
#include <thread>
#include <future>
#include <exception>
#include <stdexcept>
#include <utility>

int main() {
    std::packaged_task<int(int, int)> task1([](int a, int b) {
        return a + b;
    });

    std::future<int> future1 = task1.get_future();
    std::thread worker1(std::move(task1), 20, 30);
    try {
        int result1 = future1.get();
        std::cout << "Result: " << result1 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    worker1.join();

    // std::packaged_task 会自动捕获它所执行的任务抛出的异常。
    std::packaged_task<int()> task2([]() -> int {
        throw std::runtime_error("GPU task failed");
    });
    std::future<int> future2 = task2.get_future();
    std::thread worker2(std::move(task2));
    try {
        int result2 = future2.get();
        std::cout << "Result: " << result2 << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    worker2.join();
    
    return 0;
}