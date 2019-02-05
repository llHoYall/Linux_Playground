cmake_minimum_required(VERSION 3.8)

project(rmdir)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

file(GLOB SRCS
	rmdir.c
)

add_executable(${PROJECT_NAME} ${SRCS})

