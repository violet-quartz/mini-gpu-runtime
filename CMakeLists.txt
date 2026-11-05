cmake_minimum_required(VERSION 3.20)

project(mini-gpu-runtime
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(task_queue
    src/task_queue.cpp
)
add_library(thread_pool
    src/thread_pool.cpp
)
add_library(gpu_buffer
    src/gpu_buffer.cpp
)

foreach(target IN ITEMS task_queue thread_pool gpu_buffer)
    target_include_directories(${target}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
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
target_link_libraries(mini-runtime 
    PRIVATE 
    task_queue
    thread_pool
    gpu_buffer
)
target_link_libraries(test_gpu_buffer PRIVATE gpu_buffer)
add_test(NAME gpu_buffer_tests COMMAND test_gpu_buffer)

