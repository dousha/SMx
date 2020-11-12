cmake_minimum_required(VERSION 3.8)
project(smx)

add_library(smx STATIC sm3.c sm3.h sm4.c sm4.h common.c common.h)

