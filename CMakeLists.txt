cmake_minimum_required(VERSION 2.6)
project(gasmic)
#include_directories(include)
file(GLOB SOURCES "src/*.c")
#file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES})# ${HEADERS})

#target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)