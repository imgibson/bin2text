cmake_minimum_required(VERSION 3.8)

project(base85)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSONS OFF)

add_library(base85 INTERFACE
  include/bin2text/base85.h
)

target_include_directories(base85 INTERFACE
  include
)
