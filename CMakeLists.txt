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

foreach(target IN ITEMS task_queue thread_pool)
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

target_link_libraries(mini-runtime 
    PRIVATE 
    task_queue
    thread_pool
)

