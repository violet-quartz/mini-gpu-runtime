#include "cuda_event.h"
#include "cuda_stream.h"

#include <stdexcept>

CudaEvent::CudaEvent(unsigned int flags) {
    cudaError_t err = cudaEventCreateWithFlags(
        &event_, flags);
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

CudaEvent::~CudaEvent() {
    if (event_) {
        cudaEventDestroy(event_);
    }
}

CudaEvent::CudaEvent(CudaEvent&& other) noexcept 
    : event_(other.event_) {
    other.event_ = nullptr;
}

CudaEvent& CudaEvent::operator=(CudaEvent&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (event_) {
        cudaEventDestroy(event_);
    }
    event_ = other.event_;
    other.event_ = nullptr;
    return *this;
}

cudaEvent_t CudaEvent::get() const noexcept {
    return event_;
}

void CudaEvent::record(const CudaStream& stream) {
    if (event_ == nullptr) {
        throw std::logic_error("Cannot record an empty CudaEvent");
    }

    cudaStream_t sm = stream.get();
    if (sm == nullptr) {
        throw std::logic_error("Cannot record an event on an empty CudaStream");
    }

    cudaError_t err = cudaEventRecord(event_, sm);
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

bool CudaEvent::query() const {
    if (event_ == nullptr) {
        throw std::logic_error("Cannot query an empty CudaEvent");
    }
    cudaError_t err = cudaEventQuery(event_);
    if (err == cudaSuccess) {
        return true;
    } else if (err == cudaErrorNotReady) {
        return false;
    } else {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}

void CudaEvent::synchronize() const {
    if (event_ == nullptr) {
        throw std::logic_error("Cannot synchronize an empty CudaEvent");
    }
    cudaError_t err = cudaEventSynchronize(event_);
    if (err != cudaSuccess) {
        throw std::runtime_error(cudaGetErrorString(err));
    }
}