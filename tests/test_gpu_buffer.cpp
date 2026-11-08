#include "gpu_buffer.h"
#include "test_helpers.h"

#include <utility>

namespace {

void test_construct() {
    GpuBuffer buffer(1024);
    require(buffer.data() != nullptr);
    require(buffer.size() == 1024);
}

void test_empty_buffer() {
    GpuBuffer buffer(0);
    require(buffer.data() == nullptr);
    require(buffer.size() == 0);
}

void test_move_construct() {
    GpuBuffer source(1024);
    void* original = source.data();
    GpuBuffer destination(std::move(source));
    require(destination.data() == original);
    require(destination.size() == 1024);
    require(source.data() == nullptr);
    require(source.size() == 0);
}

void test_move_assignment() {
    GpuBuffer source(512);
    GpuBuffer destination(1024);
    void* original = source.data();
    destination = std::move(source);
    require(destination.data() == original);
    require(destination.size() == 512);
    require(source.data() == nullptr);
    require(source.size() == 0);
}

void test_self_move() {
    GpuBuffer buffer(512);
    void* original = buffer.data();
    GpuBuffer& alias = buffer;
    buffer = std::move(alias);
    require(buffer.data() == original);
    require(buffer.size() == 512);
}

} // namespace

int main() {
    run_test("test_construct", test_construct);
    run_test("test_empty_buffer", test_empty_buffer);
    run_test("test_move_construct", test_move_construct);
    run_test("test_move_assignment", test_move_assignment);
    run_test("test_self_move", test_self_move);
    check_cuda(cudaDeviceSynchronize());
}
