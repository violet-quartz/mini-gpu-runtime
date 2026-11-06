#include "cuda_event.h"
#include "cuda_stream.h"
#include "gpu_buffer.h"

#include <cassert>
#include <cuda_runtime.h>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <utility>

namespace {

void check_cuda(cudaError_t err) {
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

}

void test_construct() {
    CudaEvent e1;
    assert(e1.get() != nullptr);

    CudaEvent e2(cudaEventBlockingSync);
    assert(e2.get() != nullptr);
}

void test_move_construct() {
    CudaEvent e1;
    cudaEvent_t original = e1.get();

    CudaEvent e2(std::move(e1));
    assert(e2.get() == original);
    assert(e1.get() == nullptr);
}

void test_move_assignment() {
    CudaEvent e1;
    cudaEvent_t ee1 = e1.get();
    
    CudaEvent e2;
    e2 = std::move(e1);
    assert(e2.get() == ee1);
    assert(e1.get() == nullptr);
}

void test_self_move() {
    CudaEvent e1;
    cudaEvent_t ee1 = e1.get();
    e1 = std::move(e1);
    assert(e1.get() == ee1);
}

void test_query_before_record() {
    CudaEvent e;
    bool success = e.query();
    assert(success);
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

    assert(event.query());
    std::vector<float> hostData(count, -1.0f);
    check_cuda(cudaMemcpy(
        hostData.data(), 
        buffer.data(), 
        buffer.size(), 
        cudaMemcpyDeviceToHost
    ));
    for (float value: hostData) {
        assert(value == 0);
    }
}

void test_moved_from_operations() {
    CudaEvent e1;
    CudaEvent e2(std::move(e1));
    CudaStream stream;

    bool caught = false;
    try{
        (void)e1.query();
    } catch (const std::logic_error&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try{
        e1.record(stream);
    } catch (const std::logic_error&) {
        caught = true;
    }
    assert(caught);

    caught = false;
    try{
        e1.synchronize();
    } catch (const std::logic_error&) {
        caught = true;
    }
    assert(caught);

    e2.record(stream);
    e2.synchronize();
    assert(e2.query());
}

void test_record_with_moved_from_stream() {
    CudaStream stream1;
    CudaStream stream2(std::move(stream1));
    CudaEvent event;

    bool caught = false;

    try {
        event.record(stream1);
    } catch (const std::logic_error&) {
        caught = true;
    }

    assert(caught);
}

int main() {
    test_construct();
    test_move_construct();
    test_move_assignment();
    test_self_move();
    test_query_before_record();
    test_record_and_synchronize();
    test_moved_from_operations();
    test_record_with_moved_from_stream();
    return 0;
}