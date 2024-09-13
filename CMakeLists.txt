cmake_minimum_required(VERSION 3.20)

# Project definition
project(BongoJam_Script VERSION 1.0 DESCRIPTION "BongoJam Script" LANGUAGES CXX)

# Enforcing C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################### Conan Setup

# Include the conan toolchain
include (build/build/generators/conan_toolchain.cmake)

####################################### Compiler Project

# Automatically find all the source files in the source directory
file(
    GLOB_RECURSE 
    COMPILER_SOURCES 
    CONFIGURE_DEPENDS 
    "src/compiler/*.cpp" 
    "include/compiler/*.h"
    "include/LogManager.h"
    "include/Binder.h"
)

#### Library Target

# Create a static library
add_library(bongoC STATIC ${COMPILER_SOURCES})

# Specify include directories for the library
target_include_directories(bongoC PUBLIC include/compiler)

#### Executable Target

# Add executable based on source files
add_executable(bongo src/compiler/main.cpp)

# Link the library with the executable
target_link_libraries(bongo PRIVATE bongoC)

####################################### Interpreter Project

# Automatically find all the source files in the source directory
file(
    GLOB_RECURSE 
    INTERPRETER_SOURCES 
    CONFIGURE_DEPENDS 
    "src/runtime/*.cpp" 
    "include/runtime/*.h" 
    "include/LogManager.h"
    "include/Binder.h"
)

#### Library Target

# Create a static library
add_library(bongojam STATIC ${INTERPRETER_SOURCES})

# Specify include directories for the library
target_include_directories(bongojam PUBLIC include/runtime)

#### Executable Target

# Add executable based on source files
add_executable(jam src/runtime/main.cpp)

# Link the library with the executable
target_link_libraries(jam PRIVATE bongojam)

####################################### Dependency Linking

find_package(ZLIB REQUIRED)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader)

set_target_properties(${llvm_libs} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/static_libs/llvm/include"
)

target_link_libraries(bongoC PUBLIC
    ZLIB::ZLIB
    ${llvm_libs}
)
target_link_libraries(bongojam PUBLIC
    ZLIB::ZLIB
    ${llvm_libs}
)

#READ THE ANNOTATED TURING UWU