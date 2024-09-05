#pragma once


#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
#define NONLS             // All NLS defines and routines*/
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions
#include <windows.h>
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

#include <filesystem>
#include "Tools.h"

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