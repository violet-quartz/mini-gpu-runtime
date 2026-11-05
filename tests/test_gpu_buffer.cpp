#include "gpu_buffer.h"
#include <cassert>
#include <utility>
#include <cuda_runtime.h>

int main() {
    GpuBuffer buffer{1024};
    assert(buffer.data() != nullptr);
    assert(buffer.size() == 1024);

    GpuBuffer empty{0};
    assert(empty.data() == nullptr);
    assert(empty.size() == 0);

    GpuBuffer buffer2(std::move(buffer));
    assert(buffer2.data() != nullptr);
    assert(buffer2.size() == 1024);
    assert(buffer.data() == nullptr);
    assert(buffer.size() == 0);

    GpuBuffer buffer3{512};
    auto b3_ptr = buffer3.data();
    auto b3_size = buffer3.size();
    buffer2 = std::move(buffer3);
    assert(buffer2.data() == b3_ptr);
    assert(buffer2.size() == b3_size);
    assert(buffer3.data() == nullptr);
    assert(buffer3.size() == 0);

    auto b2_ptr = buffer2.data();
    auto b2_size = buffer2.size();
    buffer2 = std::move(buffer2);
    assert(buffer2.data() == b2_ptr);
    assert(buffer2.size() == b2_size);

    cudaError_t err = cudaDeviceSynchronize();
    assert(err == cudaSuccess);
    return 0;
}