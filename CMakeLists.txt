cmake_minimum_required(VERSION 3.20)

# Project definition
project(BongoJam_Script VERSION 1.0 DESCRIPTION "BongoJam Script" LANGUAGES CXX)

# Enforcing C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################### Conan Setup

# Include the conan toolchain
include (build/build/generators/conan_toolchain.cmake)

####################################### Source Files

# Automatically find all the source files in the source directory
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")

####################################### Library Target

# Create a static library
add_library(bongo STATIC ${SOURCES})

# Specify include directories for the library
target_include_directories(bongo PUBLIC include)

####################################### Executable Target

# Add executable based on source files
add_executable(${PROJECT_NAME} src/main.cpp)

# Link the library with the executable
target_link_libraries(${PROJECT_NAME} PRIVATE bongo)

####################################### Dependency Linking

# Locate Conan Packages
find_package(spdlog REQUIRED)

# Link Conan Packages
target_link_libraries(bongo PUBLIC
    spdlog::spdlog
)
