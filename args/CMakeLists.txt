cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_COMPILER clang)

project(args)

file(GLOB SRCS
	args.c
)

add_executable(${PROJECT_NAME} ${SRCS})

