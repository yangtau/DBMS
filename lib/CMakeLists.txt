cmake_minimum_required(VERSION 3.14)
project(BTree)
aux_source_directory(. DIR_LIB_SRCS)
# list(REMOVE_ITEM DIR_LIB_SRCS "matu.h")
# list(REMOVE_ITEM DIR_LIB_SRCS "test.cpp")
# add_library(BTree ${DIR_LIB_SRCS})
add_executable(Btree-Test ${DIR_LIB_SRCS})