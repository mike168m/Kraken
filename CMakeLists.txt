cmake_minimum_required(VERSION 3.13)
project(kraken C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(kraken
        #tests/gthreads/gthr.c
        kraken.h
        kraken_test_x86.c)
