cmake_minimum_required(VERSION 3.10)
project(solver)

# specify C++ 26 standard
set(CMAKE_CXX_STANDARD 26)
message("-- C++ version: ${CMAKE_CXX_STANDARD}")

# Set compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -m64")
if ("${CMAKE_BUILD_TYPE}" MATCHES release OR "${CMAKE_BUILD_TYPE}" MATCHES Release)
    message("-- Compiling in Release Mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")
elseif ("${CMAKE_BUILD_TYPE}" MATCHES debug OR "${CMAKE_BUILD_TYPE}" MATCHES Debug)
    message("-- Compiling in Debug Mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og -Wall -Wextra")
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