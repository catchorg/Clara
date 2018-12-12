cmake_minimum_required(VERSION 3.8.2)
project(Clara VERSION 1.1.5)

include(GNUInstallDirs)

add_library(Clara INTERFACE)
add_library(Clara::Clara ALIAS Clara)
target_include_directories(Clara
  INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_compile_features(Clara
  INTERFACE cxx_trailing_return_types)

set(SOURCE_FILES src/main.cpp src/ClaraTests.cpp)
add_executable(ClaraTests ${SOURCE_FILES})
target_include_directories(ClaraTests PRIVATE third_party)
target_link_libraries(ClaraTests PRIVATE Clara::Clara)

if(USE_CPP14)
    set_property(TARGET ClaraTests PROPERTY CXX_STANDARD 14)
    message(STATUS "Enabled C++14")
elseif(USE_CPP17)
    set_property(TARGET ClaraTests PROPERTY CXX_STANDARD 17)
    message(STATUS "Enabled C++17")
else(USE_CPP11)
    set_property(TARGET ClaraTests PROPERTY CXX_STANDARD 11)
    message(STATUS "Enabled C++11")
endif()

set_property(TARGET ClaraTests PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ClaraTests PROPERTY CXX_EXTENSIONS OFF)


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( ClaraTests PRIVATE -Wall -Wextra -pedantic -Werror )
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( ClaraTests PRIVATE /W4 /WX )
endif()

if (ENABLE_COVERAGE)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
    find_package(codecov)
    add_coverage(ClaraTests)
    list(APPEND LCOV_REMOVE_PATTERNS "/usr/")
    coverage_evaluate()
endif()

include(CTest)
add_test(NAME RunTests COMMAND $<TARGET_FILE:ClaraTests>)

set(target_export_name "${PROJECT_NAME}Targets")
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(version_config "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake")

# install headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/single_include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# install project config
install(TARGETS Clara EXPORT ${target_export_name})
install(EXPORT ${target_export_name}
  NAMESPACE Clara::
  DESTINATION ${config_install_dir}
  FILE "${PROJECT_NAME}Config.cmake")

# install version config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${version_config}
  COMPATIBILITY SameMajorVersion)
install(FILES ${version_config} DESTINATION ${config_install_dir})
