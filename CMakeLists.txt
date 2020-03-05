cmake_minimum_required(VERSION 3.1)
project(gasmic C)

include(CheckIncludeFile)
function(CHECK_AND_ADD_HEADER PROJECT HEADER VAR)
    check_include_file(${HEADER} ${VAR})
    if(${VAR})
        target_compile_definitions(${PROJECT} PRIVATE ${VAR})
    endif()
endfunction()

include(CheckTypeSize)
function(CHECK_AND_ADD_TYPE PROJECT TYPE VAR_SUFFIX BUILTIN)
	if(${BUILTIN})
		check_type_size(${TYPE} ${VAR_SUFFIX} BUILTIN_TYPES_ONLY LANGUAGE C)
	else()
		check_type_size(${TYPE} ${VAR_SUFFIX})
	endif()
	if(${HAVE_${VAR_SUFFIX}})
		target_compile_definitions(${PROJECT} PRIVATE HAVE_${VAR_SUFFIX} SIZEOF_${VAR_SUFFIX}=${VAR_SUFFIX})
	endif()
endfunction()

#include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE LOCAL_HEADERS "src/*.h")
file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES} ${LOCAL_HEADERS} ${HEADERS} src/lang.c src/lang.h)

check_and_add_header(gasmic inttypes.h HAVE_INTTYPES_H)
check_and_add_type(gasmic "size_t" SIZE_T NO)
check_and_add_type(gasmic "unsigned long int" UL YES)
check_and_add_type(gasmic "unsigned long long int" ULL YES)

#target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS gasmic DESTINATION ${CMAKE_INSTALL_PREFIX})

set_property(TARGET gasmic PROPERTY C_STANDARD 90)
set_property(TARGET gasmic PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET gasmic PROPERTY C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
