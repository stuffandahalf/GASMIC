cmake_minimum_required(VERSION 3.1)
project(gasmic C)

#include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE LOCAL_HEADERS "src/*.h")
file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES} ${LOCAL_HEADERS} ${HEADERS})

set_property(TARGET gasmic PROPERTY C_STANDARD 99)
set_property(TARGET gasmic PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET gasmic PROPERTY C_EXTENSIONS OFF)

#target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS gasmic DESTINATION ${CMAKE_INSTALL_PREFIX})