cmake_minimum_required(VERSION 3.1)
project(gasmic C)

include(CheckIncludeFile)
function(CHECK_AND_ADD_HEADER HEADER VAR)
    check_include_file(${HEADER} ${VAR})
    if (${VAR})
        target_compile_definitions(gasmic PRIVATE ${VAR})
    endif()
endfunction()

#include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE LOCAL_HEADERS "src/*.h")
file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES} ${LOCAL_HEADERS} ${HEADERS} src/lang.c src/lang.h)

set_property(TARGET gasmic PROPERTY C_STANDARD 90)
set_property(TARGET gasmic PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET gasmic PROPERTY C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

check_and_add_headers(inttypes.h HAVE_INTTYPES_H)

#target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS gasmic DESTINATION ${CMAKE_INSTALL_PREFIX})
