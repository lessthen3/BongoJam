#pragma once

#include <filesystem>
#include "Tools.h"

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
        cout << "Was not able to set console mode to allow windows to display ANSI escape codes" << "\n";
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
        LoadBindedLibrary(const string& fp_Path)
    {
        DYNLIB_HANDLE f_Handle;

        if (fs::exists(fp_Path) && fs::is_regular_file(fp_Path))
        {
            f_Handle = DYNLIB_LOAD(fp_Path.c_str());
            LogManager::MainLogger().Debug("Successfully located Cpp DLL at: " + fp_Path, "Binder");
        }

        else
        {
            LogManager::MainLogger().Error("Failed to locate Cpp DLL at: " + fp_Path, "Binder");
            return;
        }

        if (!f_Handle)
        {
            LogManager::MainLogger().Error("Failed to load Cpp Library at path: " + fp_Path, "Binder");
            return;
        }
        else
        {
            LogManager::MainLogger().Debug("Successfully loaded Cpp Library at: " + fp_Path, "Binder");
        }

        auto f_CreateFunc = (CreateCppLibraryFunc)DYNLIB_GETSYM(f_Handle, "CreateCppLibrary");
        auto f_DestroyFunc = (DestroyCppLibraryFunc)DYNLIB_GETSYM(f_Handle, "DestroyCppLibrary");

        if (!f_CreateFunc)
        {
            LogManager::MainLogger().Error("Failed to find CreateCppLibrary() functions in: " + fp_Path, "Binder");
            DYNLIB_UNLOAD(f_Handle);
            return;
        }
        else if (!f_DestroyFunc)
        {
            LogManager::MainLogger().Error("Failed to find DestroyCppLibrary() functions in: " + fp_Path, "Binder");
            DYNLIB_UNLOAD(f_Handle);
            return;
        }
        else
        {
            LogManager::MainLogger().Debug("Successfully located CreateCppLibrary() and DestroyCppLibrary() functions in: " + fp_Path, "Binder");
        }

        unique_ptr<CppLibrary, DestroyCppLibraryFunc> plugin(f_CreateFunc(), f_DestroyFunc); //creates smrt poiner with destructor tied to it;
        CURRENTLY_LOADED_LIBRARIES.emplace_back(move(plugin));

        LIBRARY_HANDLES.push_back(f_Handle);
    }

}