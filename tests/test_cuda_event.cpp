#include "cuda_event.h"
#include "cuda_stream.h"
#include "gpu_buffer.h"

#include <cassert>
#include <cuda_runtime.h>
#include <cstddef>

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

void test_move_constructor() {
    CudaEvent e1;
    cudaEvent_t original = e1.get();

    CudaEvent e2(std::move(e1));
    assert(e2.get() == original);
    assert(e1.get() == nullptr);
}

void test_move_assignment() {
    CudaEvent e1;
    cudaEvent_t ee1 = d1.get();
    
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
    std::vector<float> hostData(count);
    check_cuda(cudaMemcpy(
        hostData.data(), 
        buffer.data(), 
        count, 
        cudaMemcpyDeviceToHost
    ));
    for (float value: data) {
        assert(value == 0);
    }
}

void test_moved_from_operations() {
    CudaEvent e1;
    CudaEvent e2(std::move(e1));
    CudaStream stream;

    bool caught = false;
    try{
        bool success = e1.query();
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
    bool res = e2.query();
    e2.synchronize();
}

int main() {
    test_construct();
    test_move_construct();
    test_move_assignment();
    test_self_move();
    test_record_and_synchronize();
    test_moved_from_operations();
    return 0;
}