#include "cuda_stream.h"
#include "gpu_buffer.h"

#include <cassert>
#include <cuda_runtime.h>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

void check_cuda(cudaError_t err) {
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

}

void test_construct() {
    CudaStream stream;
    const cudaStream_t original = stream.get();
    assert(original != nullptr);

    CudaStream stream2(std::move(stream));
    assert(stream2.get() == original);
    assert(stream.get() == nullptr);
}

void test_move_assignment() {
    CudaStream stream1;
    cudaStream_t s1 = stream1.get();
    CudaStream stream2;
    cudaStream_t s2 = stream2.get();

    // test move assignment
    stream2 = std::move(stream1);
    assert(stream2.get() == s1);
    assert(stream1.get() == nullptr);

    // test self move assignment
    stream2 = std::move(stream2);
    assert(stream2.get() == s1);
    assert(s2 != nullptr);

    // test move assignment to empty
    CudaStream source;
    CudaStream destination(std::move(source));
    cudaStream_t original = destination.get();
    source = std::move(destination);
    assert(source.get() == original);
    assert(destination.get() == nullptr);
}

void test_synchronize() {
    CudaStream stream;

    const std::size_t count = 1024;
    const std::size_t bytes = count * sizeof(float);
    GpuBuffer buffer{bytes};
    check_cuda(cudaMemsetAsync(buffer.data(), 0, bytes, stream.get()));

    stream.synchronize();

    std::vector<float> hostData(count);
    check_cuda(cudaMemcpy(hostData.data(), buffer.data(), bytes, cudaMemcpyDeviceToHost));
    for (float value : hostData) {
        assert(value == 0);
    }
}

void test_moved_from_synchronize() {
    CudaStream stream1;
    CudaStream stream2(std::move(stream1));

    bool caught = false;
    try{
        stream1.synchronize();
    } catch (const std::logic_error&) {
        caught = true;
    }

    assert(caught);
    stream2.synchronize();
}

int main() {
    test_construct();
    test_move_assignment();
    test_synchronize();
    return 0;
}