cmake_minimum_required(VERSION 3.10)
project(MultiThreadingDecoder)

set(CMAKE_CXX_STANDARD 14)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(MultiThreadingDecoder main.cpp)

target_link_libraries(MultiThreadingDecoder Threads::Threads)
