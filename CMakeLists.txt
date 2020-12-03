cmake_minimum_required(VERSION 3.8)
project(smx)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_library(smx STATIC sm3.c sm3.h sm4.c sm4.h common.c common.h bigint.c bigint.h sm2.c sm2.h)

add_executable(smxtest sm3.h sm4.h common.h test.c)
target_link_libraries(smxtest PRIVATE smx)

add_executable(bitest bigint.c bigint.h bitest.c)
add_executable(ectest sm2.h sm2.c ectest.c bigint.h bigint.c)
