cmake_minimum_required(VERSION 3.8)
project(smx)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_library(smx STATIC sm3.c sm3.h sm4.c sm4.h common.c common.h)

add_executable(smxtest sm3.h sm4.h common.h test.c)
target_link_libraries(smxtest PRIVATE smx)

