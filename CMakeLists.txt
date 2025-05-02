cmake_minimum_required(VERSION 3.20)

# Project definition
project(BongoJam_Script VERSION 1.0 DESCRIPTION "BongoJam Script" LANGUAGES CXX)

# Enforcing C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################### Check For No Config Set

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "No build type specified, ending build. CMAKE WILL NOW EXIT")
endif()

####################################### Enforce 64-bit Requirement

if(NOT ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    message(FATAL_ERROR "Peach-E only supports 64-bit platforms.")
endif()

####################################### Conan Toolchain Setup

# Try to find `conan_toolchain.cmake` inside the build directory automatically
file(GLOB_RECURSE CONAN_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/**/conan_toolchain.cmake")

if(EXISTS "${CONAN_TOOLCHAIN_FILE}")
    message(STATUS "Using Conan toolchain: ${CONAN_TOOLCHAIN_FILE}")
    include(${CONAN_TOOLCHAIN_FILE})
else()
    message(FATAL_ERROR "Error: conan_toolchain.cmake not found. Run `conan install` first.")
endif()

####################################### Compiler Project

# Automatically find all the source files in the source directory
file(
    GLOB_RECURSE 
    COMPILER_SOURCES 
    CONFIGURE_DEPENDS 
    "src/compiler/*.cpp" 
    "include/compiler/*.h"
    "include/Logger.h"
    "include/Binder.h"
    "include/Opcodes.h"
)

# Remove main from bongoC for embedding the compiler into C++
list(REMOVE_ITEM COMPILER_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/main.cpp"
)

#### Library Target

# Create a static library
add_library(bongoC STATIC 

    ${COMPILER_SOURCES}
)

# Specify include directories for the library
target_include_directories(bongoC PUBLIC include/compiler)

####################################### Interpreter Project

# Automatically find all the source files in the source directory
file(
    GLOB_RECURSE 
    INTERPRETER_SOURCES 
    CONFIGURE_DEPENDS 
    "src/runtime/*.cpp" 
    "include/runtime/*.h" 
    "include/Logger.h"
    "include/Binder.h"
    "include/Opcodes.h"
)

#### Library Target

# Create a static library
add_library(bongo_runtime STATIC ${INTERPRETER_SOURCES})

# Specify include directories for the library
target_include_directories(bongo_runtime PUBLIC include/runtime)

#### Executable Target for CLI

add_executable(bongo 
    
    include/ConfigManager.h
    src/compiler/main.cpp
)

# Link the libraries with bongo CLI
target_link_libraries(bongo PRIVATE 
    bongoC
    bongo_runtime
)

####################################### Dependency Linking

find_package(ZLIB REQUIRED)

target_link_libraries(bongoC PUBLIC
    ZLIB::ZLIB
)
target_link_libraries(bongo_runtime PUBLIC
    ZLIB::ZLIB
)

####################################### Set Startup Project (Visual Studio & Xcode)

# Set bongo CLI as the default scheme in Xcode

if(CMAKE_GENERATOR MATCHES "Xcode")
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
    set(CMAKE_XCODE_SCHEME_EXECUTABLE bongo)
    message(STATUS "Setting bongo as the default startup project for Xcode")
endif()

# Set bongo CLI as the startup project for Visual Studio

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT bongo)
    message(STATUS "Setting bongo as the default startup project for VS2022")
endif()