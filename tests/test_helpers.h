#pragma once
#include <cuda_runtime.h>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

template <typename Function>
void run_test(const char* name, Function function) {
    std::cout << name << "..." << std::endl;
    function();
}

// Keep checks active in Release builds as well.
inline void require(bool condition) {
    if (!condition) {
        throw std::runtime_error("Test check failed");
    }
}

inline void check_cuda(cudaError_t error) {
    if (error != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(error));
    }
}

template <typename Function>
void expect_logic_error(Function&& operation) {
    try {
        operation();
    } catch (const std::logic_error&) {
        return;
    }
    throw std::runtime_error("Expected std::logic_error");
}

// Precondition: task.wait() has completed successfully.
template <typename Task>
void check_bytes(const Task& task, unsigned char expected) {
    std::vector<unsigned char> host(task.size());
    check_cuda(cudaMemcpy(host.data(), task.data(), task.size(),
                          cudaMemcpyDeviceToHost));
    for (unsigned char value : host) {
        require(value == expected);
    }
}

} // namespace
