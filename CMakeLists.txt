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

target_include_directories(task_queue
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(mini-runtime
    src/main.cpp
)

add_executable(future-practice
    src/future.cpp
)

target_link_libraries(mini-runtime PRIVATE task_queue)

