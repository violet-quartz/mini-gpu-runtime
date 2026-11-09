#include <iostream>
#include <thread>
#include <future>
#include <exception>
#include <stdexcept>

int main() {
    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread worker([&promise] {
       try {
        throw std::runtime_error("GPU task failed");
       } catch (...) {
        promise.set_exception(std::current_exception());
       }
    });
    try {
        int value = future.get();
        std::cout << value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << '\n';
    }
    worker.join();
    return 0;
}