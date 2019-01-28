cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_COMPILER clang)

project(cat)

file(GLOB SRCS
	cat.c
)

add_executable(${PROJECT_NAME} ${SRCS})

