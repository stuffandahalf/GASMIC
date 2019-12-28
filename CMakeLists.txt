cmake_minimum_required(VERSION 3.1)
project(gasmic C)

#include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE LOCAL_HEADERS "src/*.h")
file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES} ${LOCAL_HEADERS} ${HEADERS} src/lang.c src/lang.h)

set_property(TARGET gasmic PROPERTY C_STANDARD 90)
set_property(TARGET gasmic PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET gasmic PROPERTY C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

#target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
