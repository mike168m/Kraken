cmake_minimum_required(VERSION 3.0)


project(kraken C)


set(CMAKE_C_STANDARD 11)


include_directories(.)


add_executable( kraken_demo
                kraken.h 
                kraken_demo.c )


add_executable( kraken_test
                kraken.h
                kraken_test.c )


