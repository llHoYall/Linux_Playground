cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_COMPILER clang)

project(head)

file(GLOB SRCS
	head.c
)

add_executable(${PROJECT_NAME} ${SRCS})

