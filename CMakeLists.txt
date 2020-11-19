cmake_minimum_required(VERSION 3.17)
project(CS3841_Lab6 C)

set(CMAKE_C_STANDARD 11)

find_package( Threads )

include_directories(.)

add_executable(CS3841_Lab6
        memory_manager.c
        memory_manager.h
        pthread_testmemmgr.c
        testmemmgr.c)

target_link_libraries(CS3841_Lab6 rt ${CMAKE_THREAD_LIBS_INIT})