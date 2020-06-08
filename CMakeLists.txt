cmake_minimum_required(VERSION 2.8.11)
project(GASMIC C)

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
		target_compile_definitions(${PROJECT} PRIVATE HAVE_${VAR_SUFFIX} SIZEOF_${VAR_SUFFIX}=${${VAR_SUFFIX}})
	endif()
endfunction()

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE LOCAL_HEADERS "src/*.h")
file(GLOB HEADERS "include/*.h")

add_executable(gasmic ${SOURCES} ${LOCAL_HEADERS} ${HEADERS})

if(MSVC OR CMAKE_GENERATOR MATCHES "Visual Studio")
	target_compile_definitions(gasmic PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

set_property(TARGET gasmic PROPERTY C_STANDARD 90)
#set_property(TARGET gasmic PROPERTY C_STANDARD_REQUIRED ON)
#set_property(TARGET gasmic PROPERTY C_EXTENSIONS OFF)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

check_and_add_header(gasmic inttypes.h HAVE_INTTYPES_H)
check_and_add_header(gasmic unistd.h HAVE_UNISTD_H)
check_and_add_header(gasmic fcntl.h HAVE_FCNTL_H)
check_and_add_header(gasmic sys/stat.h HAVE_SYS_STAT_H)
check_and_add_type(gasmic "size_t" SIZE_T NO)
check_and_add_type(gasmic "unsigned long int" UL YES)
check_and_add_type(gasmic "unsigned long long int" ULL YES)

install(TARGETS gasmic DESTINATION ${CMAKE_INSTALL_PREFIX})

