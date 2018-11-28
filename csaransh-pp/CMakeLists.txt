cmake_minimum_required(VERSION 2.8)
project(csaranch-pp)

file(GLOB SOURCES src/*.cpp)

add_executable(csaransh-pp ${SOURCES})

target_compile_options(csaransh-pp PUBLIC -std=c++14 -Wall -O3)

target_include_directories(csaransh-pp PUBLIC include)

install(TARGETS csaransh-pp DESTINATION bin)
