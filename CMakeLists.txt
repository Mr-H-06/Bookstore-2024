cmake_minimum_required(VERSION 3.28)
project(Bookstore_2024)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(code
    main.cpp
    memoryriver.hpp
    block.hpp
    block.cpp)
