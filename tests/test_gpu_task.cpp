#include "test_helpers.h"
#include "gpu_task.h"
#include "cuda_stream.h"

#include <cuda_runtime.h>
#include <cstddef>
#include <utility>
#include <vector>

namespace {

// Precondition: task.wait() has completed successfully.
void check_bytes(const GpuTask& task, unsigned char expected) {
    std::vector<unsigned char> host(task.size());
    check_cuda(cudaMemcpy(host.data(), task.data(), task.size(),
                          cudaMemcpyDeviceToHost));
    for (unsigned char value : host) {
        require(value == expected);
    }
}

void test_construct() {
    GpuTask task(4096);
    require(task.size() == 4096);
    require(task.data() != nullptr);
}

void test_unsubmitted_operations() {
    GpuTask task(4096);
    expect_logic_error([&] { (void)task.query(); });
    expect_logic_error([&] { task.wait(); });
}

void test_submit_and_wait() {
    CudaStream stream;
    GpuTask task(4096);
    task.submit_memset(stream, 0xA5);
    task.wait();
    require(task.query());
    check_bytes(task, 0xA5);
}

void test_move_construct() {
    CudaStream stream;
    GpuTask source(4096);
    source.submit_memset(stream, 0x37);
    void* original = source.data();

    GpuTask destination(std::move(source));
    require(destination.data() == original);
    require(destination.size() == 4096);
    require(source.data() == nullptr);
    require(source.size() == 0);
    destination.wait();
    require(destination.query());
    check_bytes(destination, 0x37);
    expect_logic_error([&] { source.submit_memset(stream, 0); });
}

void test_move_assignment() {
    CudaStream source_stream;
    CudaStream destination_stream;
    GpuTask source(4096);
    GpuTask destination(8192);
    source.submit_memset(source_stream, 0xC3);
    destination.submit_memset(destination_stream, 0x19);
    void* original = source.data();

    destination = std::move(source);
    require(destination.data() == original);
    require(destination.size() == 4096);
    require(source.data() == nullptr);
    require(source.size() == 0);
    destination.wait();
    require(destination.query());
    check_bytes(destination, 0xC3);
    expect_logic_error([&] { source.submit_memset(source_stream, 0); });
    destination_stream.synchronize();
    check_cuda(cudaGetLastError());

    GpuTask c(2048);
    void* original_c = c.data();
    source = std::move(c);
    require(source.data() == original_c);
    require(source.size() == 2048);
    require(c.data() == nullptr);
    require(c.size() == 0);
}

void test_self_move() {
    CudaStream stream;
    GpuTask task(4096);
    task.submit_memset(stream, 0x5A);
    void* original = task.data();
    // Use an alias so compilers do not warn about intentional self-move.
    GpuTask& alias = task;
    task = std::move(alias);
    require(task.data() == original);
    require(task.size() == 4096);
    task.wait();
    require(task.query());
    check_bytes(task, 0x5A);
    expect_logic_error([&] { task.submit_memset(stream, 0); });
}

void test_destroy_submitted_task() {
    CudaStream stream;
    {
        GpuTask task(4096);
        task.submit_memset(stream, 0x7E);
    }
    stream.synchronize();
    check_cuda(cudaDeviceSynchronize());
    check_cuda(cudaGetLastError());
}

void test_wait_after_stream_destruction() {
    GpuTask task(4096);
    {
        CudaStream stream;
        task.submit_memset(stream, 0x81);
    }
    task.wait();
    require(task.query());
    check_bytes(task, 0x81);
}

void test_repeated_submit() {
    CudaStream stream;
    GpuTask task(4096);
    task.submit_memset(stream, 0x42);
    expect_logic_error([&] { task.submit_memset(stream, 0x99); });
    task.wait();
    expect_logic_error([&] { task.submit_memset(stream, 0x99); });
    require(task.query());
    check_bytes(task, 0x42);
}

void test_invalid_submit() {
    // Case 1: Zero-size buffer
    {
        GpuTask task(0);
        CudaStream stream;
        expect_logic_error([&] {
            task.submit_memset(stream, 0);
        });
    }

    // Case 2: Moved-from stream
    {
        CudaStream stream;
        CudaStream moved(std::move(stream));
        GpuTask task(4096);
        expect_logic_error([&] {
            task.submit_memset(stream, 0);
        });
    }
}

} // namespace

int main() {
    run_test("test_construct", test_construct);
    run_test("test_unsubmitted_operations", test_unsubmitted_operations);
    run_test("test_submit_and_wait", test_submit_and_wait);
    run_test("test_move_construct", test_move_construct);
    run_test("test_move_assignment", test_move_assignment);
    run_test("test_self_move", test_self_move);
    run_test("test_destroy_submitted_task", test_destroy_submitted_task);
    run_test("test_wait_after_stream_destruction", test_wait_after_stream_destruction);
    run_test("test_repeated_submit", test_repeated_submit);
    run_test("test_invalid_submit", test_invalid_submit);
}
