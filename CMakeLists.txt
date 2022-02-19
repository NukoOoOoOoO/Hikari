cmake_minimum_required(VERSION 3.15)

# project name
project(Hikari)

# C++ 20 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp"
"test/test.cpp")

add_executable(Hikari ${SOURCES})
set_property(TARGET PROPERTY CXX_STANDARD 23)