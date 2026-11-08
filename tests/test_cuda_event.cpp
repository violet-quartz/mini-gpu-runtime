#include "test_helpers.h"
#include "cuda_event.h"
#include "cuda_stream.h"
#include "gpu_buffer.h"

#include <cuda_runtime.h>
#include <cstddef>
#include <vector>
#include <utility>

namespace {

void test_construct() {
    CudaEvent e1;
    require(e1.get() != nullptr);

    CudaEvent e2(cudaEventBlockingSync);
    require(e2.get() != nullptr);
}

void test_move_construct() {
    CudaEvent e1;
    cudaEvent_t original = e1.get();

    CudaEvent e2(std::move(e1));
    require(e2.get() == original);
    require(e1.get() == nullptr);
}

void test_move_assignment() {
    CudaEvent e1;
    cudaEvent_t ee1 = e1.get();

    CudaEvent e2;
    e2 = std::move(e1);
    require(e2.get() == ee1);
    require(e1.get() == nullptr);
}

void test_self_move() {
    CudaEvent e1;
    cudaEvent_t ee1 = e1.get();
    CudaEvent& alias = e1;
    e1 = std::move(alias);
    require(e1.get() == ee1);
}

void test_query_before_record() {
    CudaEvent e;
    bool success = e.query();
    require(success);
}

void test_record_and_synchronize() {
    CudaStream stream;
    CudaEvent event;
    std::size_t count = 1024;
    GpuBuffer buffer{count * sizeof(float)};

    check_cuda(cudaMemsetAsync(
        buffer.data(),
        0,
        buffer.size(),
        stream.get()
    ));

    event.record(stream);
    event.synchronize();

    require(event.query());
    std::vector<float> hostData(count, -1.0f);
    check_cuda(cudaMemcpy(
        hostData.data(),
        buffer.data(),
        buffer.size(),
        cudaMemcpyDeviceToHost
    ));
    for (float value: hostData) {
        require(value == 0);
    }
}

void test_moved_from_operations() {
    CudaEvent e1;
    CudaEvent e2(std::move(e1));
    CudaStream stream;

    expect_logic_error([&] { (void)e1.query(); });
    expect_logic_error([&] { e1.record(stream); });
    expect_logic_error([&] { e1.synchronize(); });

    e2.record(stream);
    e2.synchronize();
    require(e2.query());
}

void test_record_with_moved_from_stream() {
    CudaStream stream1;
    CudaStream stream2(std::move(stream1));
    CudaEvent event;

    expect_logic_error([&] { event.record(stream1); });
}

} // namespace

int main() {
    run_test("test_construct", test_construct);
    run_test("test_move_construct", test_move_construct);
    run_test("test_move_assignment", test_move_assignment);
    run_test("test_self_move", test_self_move);
    run_test("test_query_before_record", test_query_before_record);
    run_test("test_record_and_synchronize", test_record_and_synchronize);
    run_test("test_moved_from_operations", test_moved_from_operations);
    run_test("test_record_with_moved_from_stream", test_record_with_moved_from_stream);
}
