cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

add_executable(main main.cpp)
target_link_libraries( main ${Boost_LIBRARIES} Threads::Threads )