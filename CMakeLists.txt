cmake_minimum_required(VERSION 3.6.2)
project(Clara)

set(SOURCE_FILES src/main.cpp src/ClaraTests.cpp src/Surrogate.cpp include/clara.hpp)
include_directories( include third_party )
add_executable(Clara ${SOURCE_FILES})

set_property(TARGET Clara PROPERTY CXX_STANDARD 11)
set_property(TARGET Clara PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET Clara PROPERTY CXX_EXTENSIONS OFF)


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( Clara PRIVATE -Wall -Wextra )
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( Clara PRIVATE /W4 /WX )
endif()


include(CTest)
add_test(NAME RunTests COMMAND $<TARGET_FILE:Clara>)
