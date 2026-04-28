####################################### Enforce CMake Version

cmake_minimum_required(VERSION 4.2.0)

####################################### Compilation Options

option(BONGO_BUILD_SHARED "build as static lib by default" OFF) 
option(USE_STATIC_MSVCRT "default use static crt" OFF) 

option(WINDOWS_SHARED_EXPORT_ALL_SYMBOLS "by default symbols arent exported on shared builds" OFF)

####################################### Project definition

project(BongoJam_Script VERSION 1.0 DESCRIPTION "BongoJam Script" LANGUAGES CXX C)

####################################### Enforcing C++20 standard

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

####################################### Enforce C17 Standard

set(CMAKE_C_STANDARD 17) #needed for box2d owo
set(CMAKE_C_STANDARD_REQUIRED ON)

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

####################################### Bongo Lang Project

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

if(BONGO_BUILD_SHARED)
    target_compile_definitions(bongo_core PRIVATE 
        BONGO_SHARED
    )   
else()
    target_compile_definitions(bongo_core PRIVATE 
        BONGO_STATIC
    )   
endif()

####################################### Source includes for BongoJam Lang

file(GLOB FMT_SOURCES
    third_party/fmt/format.cc
    third_party/fmt/os.cc
)

file(GLOB LZ4_SOURCES
    third_party/lz4/*.c
)

file(GLOB_RECURSE ASMJIT_SOURCES
    third_party/asmjit/*.cpp
)

target_sources(bongo_core PRIVATE
    ${LZ4_SOURCES}
    ${FMT_SOURCES}
    ${ASMJIT_SOURCES}
)

target_include_directories(bongo_core SYSTEM PUBLIC
    third_party/ #supported across the board owo ily guys ~~nyah
)

source_group("third_party/lz4" FILES ${LZ4_SOURCES})
source_group("third_party/fmt" FILES ${FMT_SOURCES})
source_group("third_party/asmjit" FILES ${ASMJIT_SOURCES})

# disable third party warnings OwO, idk just trusting em
if(MSVC)
    set(WARNING_OFF_FLAG "/W0")
else()
    set(WARNING_OFF_FLAG "-w") #just assuming clang or gcc if not msvc
endif()

set_source_files_properties(
    ${LZ4_SOURCES}
    ${FMT_SOURCES}
    ${ASMJIT_SOURCES}
    PROPERTIES COMPILE_FLAGS ${WARNING_OFF_FLAG}
)

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

set_target_properties(bongo PROPERTIES POSITION_INDEPENDENT_CODE ON) #make PIC friendly UwU

####################################### Compiler Options and Definitions

target_compile_definitions(bongo_core PUBLIC 
    $<$<CONFIG:Debug>:BONGO_DEBUG>
)

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

        target_compile_options(bongo_core PUBLIC #for fmt
            /utf-8
        )
        target_compile_options(bongo_core PRIVATE
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:${MSVCRT_RELEASE_TYPE}>
        ) 
        target_compile_options(bongo PRIVATE
            $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:${MSVCRT_RELEASE_TYPE}>
        ) 
endif()

####################################### Master snuggle list for platform detection UwU (my chatgpt is bricked af)
# daddy guarantees exactly ONE of these is always defined per build so no ambiguity 🍑
# daddy guaranteed so ig it works OwO

peach_apply_platform_definitions(bongo_core PUBLIC)

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