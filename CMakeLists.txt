cmake_minimum_required(VERSION 3.10)
project(Semaphores_In_Cpp)

set(CMAKE_CXX_STANDARD 14)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(Semaphores_In_Cpp main.cpp)

target_link_libraries(Semaphores_In_Cpp Threads::Threads)
