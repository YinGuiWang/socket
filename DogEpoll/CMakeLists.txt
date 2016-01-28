cmake_minimum_required(VERSION 3.3)
project(DogEpoll)
project(thread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

set(SOURCE_FILES main.cpp)
add_executable(DogEpoll ${SOURCE_FILES})