cmake_minimum_required(VERSION 3.27.0)

project(shmq)

message("System name: ${CMAKE_SYSTEM_NAME}")
message("System version: ${CMAKE_SYSTEM_VERSION}")
message("Processor: ${CMAKE_SYSTEM_PROCESSOR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_FLAGS "")
set(CMAKE_C_FLAGS_RELEASE "")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS_DEBUG "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_SUBDIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${BUILD_SUBDIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_SUBDIRECTORY})

set(SUPPORTED_BUILD_TYPE "Debug" "Release")
set(BUILD_TYPE "" CACHE STRING "build type")

if ("${BUILD_TYPE}" STREQUAL "")
    set(BUILD_TYPE "$ENV{BUILD_TYPE}")
endif()

message(STATUS "Supported BUILD_TYPE (${SUPPORTED_BUILD_TYPE})")
message(STATUS "BUILD_TYPE: ${BUILD_TYPE}")
if(NOT "${BUILD_TYPE}" IN_LIST SUPPORTED_BUILD_TYPE)
    message(FATAL_ERROR "BUILD_TYPE not recognized.")
endif()
set(CMAKE_BUILD_TYPE ${ON_BUILD_TYPE})

message(${THIRD_PARTY_FOLDER})

SET(TARGET_LIST)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/samples)
message("TARGET_LIST: ${TARGET_LIST}")
foreach(TARGET IN LISTS TARGET_LIST)
    message(STATUS "Adding compile and link options for target: ${TARGET}")
    target_compile_options(
        ${TARGET} PUBLIC
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Wpedantic
        -march=x86-64
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-DDEBUG>
        $<$<CONFIG:Release>:-DNDEBUG>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-flto>
    )
    target_link_options(
        ${TARGET} PUBLIC
        $<$<CONFIG:Debug>:-fsanitize=undefined>
        $<$<CONFIG:Release>:-flto>
    )
endforeach()

add_custom_target(TARGETS_TO_BUILD ALL)
add_dependencies(
    TARGETS_TO_BUILD
    ${TARGET_LIST}
)
