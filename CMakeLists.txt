cmake_minimum_required(VERSION 3.7)
project(Clara)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/main.cpp src/ClaraTests.cpp src/Surrogate.cpp include/clara.hpp)
include_directories( include third_party )
add_executable(Clara ${SOURCE_FILES})

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( Clara PRIVATE -Wall -Wextra )
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( Clara PRIVATE /W4 /WX )
endif()
