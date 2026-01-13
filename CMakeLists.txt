####################################### Enforce CMake Version

cmake_minimum_required(VERSION 4.2.0)

####################################### Compilation Options

option(USE_STATIC_MSVCRT "default use static crt" OFF) 
option(BONGO_BUILD_SHARED "build as static lib by default" OFF) 
option(WINDOWS_SHARED_EXPORT_ALL_SYMBOLS "by default symbols arent exported on shared builds" OFF)

####################################### Project definition

project(BongoJam_Script VERSION 1.0 DESCRIPTION "BongoJam Script" LANGUAGES CXX)

####################################### Enforcing C++20 standard

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################### Setting Variables

if(BONGO_BUILD_SHARED)
    set(BONGO_BUILD_TYPE SHARED)
    message(STATUS "Setting build type for compiler -> SHARED")
else()
    set(BONGO_BUILD_TYPE STATIC)
    message(STATUS "Setting build type for compiler -> STATIC")
endif()

####################################### Check For No Config Set

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "No build type specified, ending build. CMAKE WILL NOW EXIT")
endif()

####################################### Enforce 64-bit Requirement

if(NOT ${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    message(FATAL_ERROR "Peach-E only supports 64-bit platforms.")
endif()

####################################### Compiler Project

# Automatically find all the source files in the source directory
file(
    GLOB_RECURSE 
    COMPILER_INTERPRETER_SOURCES 
    CONFIGURE_DEPENDS 
    "src/bongo_lang/*.cpp" 
    "src/bongo_lang/*.h"
)

#Create compiler lib
add_library(bongo_core ${BONGO_BUILD_TYPE} 
    ${COMPILER_INTERPRETER_SOURCES}
)

set_target_properties(bongo_core PROPERTIES POSITION_INDEPENDENT_CODE ON) #make PIC friendly UwU

####################################### Executable Target for CLI Tool

#Find all sources, and check for changes every run
file(
    GLOB_RECURSE 
    BONGO_CLI_SOURCES
    CONFIGURE_DEPENDS 
    "src/cli/*.h"
    "src/cli/*.cpp"
)

add_executable(bongo 
    ${BONGO_CLI_SOURCES}
)

# Link the libraries with bongo CLI
target_link_libraries(bongo PRIVATE 
    bongo_core
)

target_include_directories(bongo PRIVATE
    ${PROJECT_SOURCE_DIR}/src/bongo_lang
)

####################################### Compiler Options and Definitions

target_compile_definitions(bongo_core PRIVATE 
    $<$<CONFIG:Debug>:BONGO_DEBUG>
)

target_compile_definitions(bongo PRIVATE 
    $<$<CONFIG:Debug>:BONGO_DEBUG>
)

if(BONGO_BUILD_SHARED)
    target_compile_definitions(bongo_server PRIVATE 
        BONGO_SHARED
    )   
endif()

if(MSVC)
    if(USE_STATIC_MSVCRT)
        set(MSVCRT_DEBUG_TYPE /MTd)
        set(MSVCRT_RELEASE_TYPE /MT)
        message(STATUS "Using Static msvcrt")
    else()
        set(MSVCRT_DEBUG_TYPE /MDd)
        set(MSVCRT_RELEASE_TYPE /MD)
        message(STATUS "Using Dynamic msvcrt")
    endif()

        target_compile_options(bongo_core PRIVATE
            $<$<CONFIG:Debug>:${MSVCRT_DEBUG_TYPE}>
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:${MSVCRT_RELEASE_TYPE}>
        ) 
        target_compile_options(bongo PRIVATE
            $<$<CONFIG:Debug>:${MSVCRT_DEBUG_TYPE}>
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:${MSVCRT_RELEASE_TYPE}>
        ) 
endif()

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