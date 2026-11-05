cmake_minimum_required(VERSION 3.20)

project(mini-gpu-runtime
    VERSION 1.0.0
    LANGUAGES CXX
)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CUDAToolkit REQUIRED)

add_library(task_queue
    src/task_queue.cpp
)
add_library(thread_pool
    src/thread_pool.cpp
)
add_library(gpu_buffer
    src/gpu_buffer.cpp
)
add_library(cuda_stream
    src/cuda_stream.cpp
)

foreach(target IN ITEMS task_queue thread_pool gpu_buffer cuda_stream)
    target_include_directories(${target}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endforeach()

target_link_libraries(thread_pool
    PUBLIC task_queue
)

foreach(target IN ITEMS gpu_buffer cuda_stream)
    target_link_libraries(${target}
        PUBLIC CUDA::cudart
    )
endforeach()

add_executable(mini-runtime
    src/main.cpp
)
add_executable(future-practice
    src/future.cpp
)
add_executable(packaged-task-practice
    src/packaged_task.cpp
)
add_executable(test_gpu_buffer
    tests/test_gpu_buffer.cpp
)
add_executable(test_cuda_stream
    tests/test_cuda_stream.cpp
)
target_link_libraries(mini-runtime 
    PRIVATE 
    thread_pool
    gpu_buffer
)
target_link_libraries(test_gpu_buffer PRIVATE gpu_buffer)
target_link_libraries(test_cuda_stream 
    PRIVATE 
    cuda_stream
    gpu_buffer
)

add_test(NAME gpu_buffer_tests COMMAND test_gpu_buffer)
add_test(NAME cuda_stream_tests COMMAND test_cuda_stream)
