/*******************************************************************
 *                                        BongoJam Script v0.3                                        
 *                           Created by Ranyodh Mandur - � 2024                            
 *                                                                                                                  
 *                         Licensed under the MIT License (MIT).                           
 *                  For more details, see the LICENSE file or visit:                     
 *                        https://opensource.org/licenses/MIT                               
 *                                                                                                                  
 *  BongoJam is an open-source scripting language compiler and interpreter 
 *              primarily intended for embedding within game engines.               
********************************************************************/
#pragma once

#include "LogManager.h"

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h>

static bool
    EnableColors()
{
    DWORD f_ConsoleMode;
    HANDLE f_OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleMode(f_OutputHandle, &f_ConsoleMode))
    {
        SetConsoleMode(f_OutputHandle, f_ConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        return true;
    }
    else
    {
        BongoJam::LogManager::Logger().LogAndPrint("Was not able to set console mode to allow windows to display ANSI escape codes", "Binder", "error");
        return false;
    }
}

#define DYNLIB_HANDLE HINSTANCE
#define DYNLIB_LOAD LoadLibraryA
#define DYNLIB_GETSYM GetProcAddress
#define DYNLIB_UNLOAD FreeLibrary
#else
#include <dlfcn.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD(path) dlopen(path, RTLD_LAZY)
#define DYNLIB_GETSYM dlsym
#define DYNLIB_UNLOAD dlclose
#endif

using namespace std;

namespace BongoJam {

    struct CppLibrary
    {
        virtual ~CppLibrary() = default;
        virtual void Initialize() = 0;
    };

    namespace fs = filesystem;

    typedef CppLibrary* (*CreateCppLibraryFunc)();
    typedef void (*DestroyCppLibraryFunc)(CppLibrary*);

    static vector<unique_ptr<CppLibrary, DestroyCppLibraryFunc>> CURRENTLY_LOADED_LIBRARIES;
    static vector<DYNLIB_HANDLE> LIBRARY_HANDLES;

    void 
        LoadBindedLibrary
        (
            const string& fp_Path
        )
    {
        DYNLIB_HANDLE f_Handle;

        if (fs::exists(fp_Path) and fs::is_regular_file(fp_Path))
        {
            f_Handle = DYNLIB_LOAD(fp_Path.c_str());
            LogManager::Logger().LogAndPrint("Successfully located Cpp DLL at: " + fp_Path, "Binder", "debug");
        }
        else
        {
            LogManager::Logger().LogAndPrint("Failed to locate Cpp DLL at: " + fp_Path, "Binder", "error");
            return;
        }

        if (!f_Handle)
        {
            LogManager::Logger().LogAndPrint("Failed to load Cpp Library at path: " + fp_Path, "Binder", "error");
            return;
        }
        else
        {
            LogManager::Logger().LogAndPrint("Successfully loaded Cpp Library at: " + fp_Path, "Binder", "debug");
        }

        auto f_CreateFunc = (CreateCppLibraryFunc)DYNLIB_GETSYM(f_Handle, "CreateCppLibrary");
        auto f_DestroyFunc = (DestroyCppLibraryFunc)DYNLIB_GETSYM(f_Handle, "DestroyCppLibrary");

        if (!f_CreateFunc)
        {
            LogManager::Logger().LogAndPrint("Failed to find CreateCppLibrary() functions in: " + fp_Path, "Binder", "error");
            DYNLIB_UNLOAD(f_Handle);
            return;
        }
        else if (!f_DestroyFunc)
        {
            LogManager::Logger().LogAndPrint("Failed to find DestroyCppLibrary() functions in: " + fp_Path, "Binder", "error");
            DYNLIB_UNLOAD(f_Handle);
            return;
        }
        else
        {
            LogManager::Logger().LogAndPrint("Successfully located CreateCppLibrary() and DestroyCppLibrary() functions in: " + fp_Path, "Binder", "debug");
        }

        unique_ptr<CppLibrary, DestroyCppLibraryFunc> plugin(f_CreateFunc(), f_DestroyFunc); //creates smrt poiner with destructor tied to it;
        CURRENTLY_LOADED_LIBRARIES.emplace_back(move(plugin));

        LIBRARY_HANDLES.push_back(f_Handle);
    }

}