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
function(CHECK_AND_ADD_TYPE PROJECT TYPE VAR)
	check_type_size(${TYPE} SIZEOF_${VAR} BUILTIN_TYPES_ONLY LANGUAGE C)
	if(${HAVE_SIZEOF_${VAR}})
		target_compile_definitions(${PROJECT} PRIVATE ${VAR})
	endif()
endfunction()

#include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE LOCAL_HEADERS "src/*.h")
file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES} ${LOCAL_HEADERS} ${HEADERS} src/lang.c src/lang.h)

check_and_add_header(gasmic inttypes.h HAVE_INTTYPES_H)

check_type_size("size_t" SIZEOF_SIZE_T)# BUILTIN_TYPES_ONLY LANGUAGE C)
check_type_size("unsigned long int" SIZEOF_UL BUILTIN_TYPES_ONLY LANGUAGE C)

#message(STATUS "SIZEOF_SIZE_T = ${SIZEOF_SIZE_T}")
#message(STATUS "SIZEOF_UL = ${SIZEOF_UL})

if(SIZEOF_SIZE_T EQUAL SIZEOF_UL)
	target_compile_definitions(gasmic PRIVATE SZuFMT="%%lu" SZxFMT="%%lx" SZXFMT="%%lX")
else()
	check_type_size("unsigned long long int" SIZEOF_ULL BUILTIN_TYPES_ONLY LANGUAGE C)
	#message(STATUS "SIZEOF_ULL = ${SIZEOF_ULL})
	if (SIZEOF_SIZE_T EQUAL SIZEOF_ULL)
		target_compile_definitions(gasmic PRIVATE SZuFMT="%%llu" SZxFMT="%%llx" SZXFMT="%%llX")
	else()
		message(SEND_ERROR "Could not determine base type of size_t, assuming int")
		target_compile_definitions(gasmic PRIVATE SZuFMT="%%u" SZxFMT="%%x" SZXFMT="%%X")
	endif()
endif()

#target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(gasmic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS gasmic DESTINATION ${CMAKE_INSTALL_PREFIX})

set_property(TARGET gasmic PROPERTY C_STANDARD 90)
set_property(TARGET gasmic PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET gasmic PROPERTY C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
