#include "test_helpers.h"
#include "cuda_stream.h"
#include "gpu_buffer.h"

#include <cuda_runtime.h>
#include <cstddef>
#include <utility>
#include <vector>

namespace {

void test_construct() {
    CudaStream stream;
    const cudaStream_t original = stream.get();
    require(original != nullptr);
}

void test_move_construct() {
    CudaStream stream;
    const cudaStream_t original = stream.get();
    CudaStream stream2(std::move(stream));
    require(stream2.get() == original);
    require(stream.get() == nullptr);
}

void test_move_assignment() {
    CudaStream stream1;
    cudaStream_t s1 = stream1.get();
    CudaStream stream2;

    // test move assignment
    stream2 = std::move(stream1);
    require(stream2.get() == s1);
    require(stream1.get() == nullptr);

    // test move assignment to empty
    CudaStream source;
    CudaStream destination(std::move(source));
    cudaStream_t original = destination.get();
    source = std::move(destination);
    require(source.get() == original);
    require(destination.get() == nullptr);
}

void test_self_move() {
    CudaStream stream;
    const cudaStream_t original = stream.get();
    CudaStream& alias = stream;
    stream = std::move(alias);
    require(stream.get() == original);
    stream.synchronize();
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
        require(value == 0);
    }
}

void test_moved_from_synchronize() {
    CudaStream stream1;
    CudaStream stream2(std::move(stream1));

    expect_logic_error([&] { stream1.synchronize(); });
    stream2.synchronize();
}

} // namespace

int main() {
    run_test("test_construct", test_construct);
    run_test("test_move_construct", test_move_construct);
    run_test("test_self_move", test_self_move);
    run_test("test_move_assignment", test_move_assignment);
    run_test("test_synchronize", test_synchronize);
    run_test("test_moved_from_synchronize", test_moved_from_synchronize);
}
