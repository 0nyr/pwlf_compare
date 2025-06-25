cmake_minimum_required(VERSION 3.10)
project(solver)

# specify C++ 26 standard
set(CMAKE_CXX_STANDARD 26)
message("-- C++ version: ${CMAKE_CXX_STANDARD}")

# Disable Nix no native
set(NIX_ENFORCE_NO_NATIVE OFF)

# allow three build‐types: Release, Debug, Fastdebug
set(VALID_BUILD_TYPES Release Debug Fastdebug)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Fastdebug CACHE STRING "Choose build type: ${VALID_BUILD_TYPES}" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# common flags
set(COMMON_FLAGS "-pthread -m64")
string(APPEND CMAKE_CXX_FLAGS        " ${COMMON_FLAGS}")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${COMMON_FLAGS}")

# sanitizer flags for plain Debug only
set(UBSAN_FLAGS "-fsanitize=float-divide-by-zero,float-cast-overflow")

# set flags based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "-- Compiling in Release Mode")
  string(APPEND CMAKE_CXX_FLAGS " -O3 -DNDEBUG -march=native -mtune=native")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "-- Compiling in Debug Mode (UBSan & logging)")
  string(APPEND CMAKE_CXX_FLAGS        " -g -Og -Wall -Wextra ${UBSAN_FLAGS}")
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${UBSAN_FLAGS}")
  # preserve full logging (no extra defines)
elseif(CMAKE_BUILD_TYPE STREQUAL "Fastdebug")
  message(STATUS "-- Compiling in Fastdebug Mode (UBSan, no logging, quick iter)")
  # a light optimization level to keep compile+link cheap
  string(APPEND CMAKE_CXX_FLAGS " -g -O1 -Wall -Wextra ${UBSAN_FLAGS}")
  string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${UBSAN_FLAGS}")
else()
  message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}. Valid types: ${VALID_BUILD_TYPES}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # used to generate compile_commands.json for VSCode intellisense/warnings

# Tell GCC to use the plugin-based linker and allow real parallel LTO
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}
  -fuse-linker-plugin
  -fuse-ld=gold"    # or -fuse-ld=lld if you prefer LLVM’s linker
)

# Find and include pybind11.
set(PYBIND11_FINDPYTHON ON)
find_package(Python  REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
pybind11_add_module(pwlf_cpp bindings.cpp main.cpp)
target_include_directories(pwlf_cpp PRIVATE ${CMAKE_SOURCE_DIR}/src)


# Add include directory.
include_directories(".")

# Create library with source codes.
add_library(
    solver 
    main.cpp
)
target_link_libraries(solver) # add compiled libs

# Create binaries.
# add_executable(main main.cpp)
# target_link_libraries(main solver)