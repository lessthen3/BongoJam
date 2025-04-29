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

#include "Logger.h"

#if defined(_WIN32) || defined(_WIN64)
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

namespace BongoJam {

    struct CppLibrary
    {
        virtual ~CppLibrary() = default;
        virtual void Initialize() = 0;
    };

    typedef CppLibrary* (*CreateCppLibraryFunc)();
    typedef void (*DestroyCppLibraryFunc)(CppLibrary*);

    static vector<unique_ptr<CppLibrary, DestroyCppLibraryFunc>> CURRENTLY_LOADED_LIBRARIES;
    static vector<DYNLIB_HANDLE> LIBRARY_HANDLES;

    void 
        LoadBindedLibrary
        (
            const string& fp_Path,
            Logger* logger
        )
    {
        DYNLIB_HANDLE f_Handle;

        if (filesystem::exists(fp_Path) and filesystem::is_regular_file(fp_Path))
        {
            f_Handle = DYNLIB_LOAD(fp_Path.c_str());
            logger->LogAndPrint("Successfully located Cpp DLL at: " + fp_Path, "Binder", Logger::LogLevel::Debug);
        }
        else
        {
            logger->LogAndPrint("Failed to locate Cpp DLL at: " + fp_Path, "Binder", Logger::LogLevel::Error);
            return;
        }

        if (!f_Handle)
        {
            logger->LogAndPrint("Failed to load Cpp Library at path: " + fp_Path, "Binder", Logger::LogLevel::Error);
            return;
        }
        else
        {
            logger->LogAndPrint("Successfully loaded Cpp Library at: " + fp_Path, "Binder", Logger::LogLevel::Debug);
        }

        auto f_CreateFunc = (CreateCppLibraryFunc)DYNLIB_GETSYM(f_Handle, "CreateCppLibrary");
        auto f_DestroyFunc = (DestroyCppLibraryFunc)DYNLIB_GETSYM(f_Handle, "DestroyCppLibrary");

        if (not f_CreateFunc)
        {
            logger->LogAndPrint("Failed to find CreateCppLibrary() functions in: " + fp_Path, "Binder", Logger::LogLevel::Error);
            DYNLIB_UNLOAD(f_Handle);
            return;
        }
        else if (not f_DestroyFunc)
        {
            logger->LogAndPrint("Failed to find DestroyCppLibrary() functions in: " + fp_Path, "Binder", Logger::LogLevel::Error);
            DYNLIB_UNLOAD(f_Handle);
            return;
        }
        else
        {
            logger->LogAndPrint("Successfully located CreateCppLibrary() and DestroyCppLibrary() functions in: " + fp_Path, "Binder", Logger::LogLevel::Debug);
        }

        unique_ptr<CppLibrary, DestroyCppLibraryFunc> plugin(f_CreateFunc(), f_DestroyFunc); //creates smrt poiner with destructor tied to it;
        CURRENTLY_LOADED_LIBRARIES.emplace_back(move(plugin));

        LIBRARY_HANDLES.push_back(f_Handle);
    }

}