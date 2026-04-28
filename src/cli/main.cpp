/**************************************************************************
 *                         BongoJam Script v0.0.1
 *                  Created by Ranyodh Mandur - 🗻2024
 *
 *                 Licensed under the MIT License (MIT).
 *            For more details, see the LICENSE file or visit:
 *                 https://opensource.org/licenses/MIT
 *
 * BongoJam is an open-source scripting language compiler and interpreter
 *        primarily intended for embedding within game engines.
**************************************************************************/
#include <string>

#include "BongoManager.h"

#include <csignal>

constexpr const int FATAL_SEGMENTATION_FAULT = -6969;

static void
    SegFaultHandler(int fp_Signal) //primitive segfault handler
{
    BONGO_PRINT_ERROR_FMT("[!]FATAL SEGMENTATION FAULT: Crash signal received {}", fp_Signal);
    // possibly notify watchdog or dump stack trace
    exit(FATAL_SEGMENTATION_FAULT); //clean exit so everything calls their destructors
}

static inline constexpr void 
    ReplaceChar(std::string* fp_String, char fp_OldChar, char fp_NewChar)
{
    for (size_t i = 0; i < fp_String->length(); ++i)
    {
        if ((*fp_String)[i] == fp_OldChar)
        {
            (*fp_String)[i] = fp_NewChar;
        }
    }
}

static std::string
    GetProjectRootDirectory(const char* fp_RootDirectory)
{
        //WARNING: WE ONLY USE THIS FOR DEVELOPMENT, FOR DEPLOYMENT WE NEED THIS DIRECTORY TO BE THE BASE DIR OF THE EXECUTABLE
    // Get the full path of the executable
    std::filesystem::path mf_ExePath = std::filesystem::absolute(fp_RootDirectory);
    std::filesystem::path f_TopLevelDir = mf_ExePath.parent_path();  // Start from the executable directory

    // Traverse upwards until we find the "Peach-E" directory
    while (not f_TopLevelDir.empty() and f_TopLevelDir.filename() != "BongoJam")
    {
        f_TopLevelDir = f_TopLevelDir.parent_path();
    }

    if (f_TopLevelDir.empty())
    {
        BONGO_PRINT("Failed to find the top-level directory 'Peach-E'!", BONGO_COL_MAGENTA);
        return "";
    }

    std::string f_ProjectRootDirectory = f_TopLevelDir.string();

    //itll just leave the string unaffected for good OS' like linux or linux im not gonna say mac beacuse that shit fucking sucks
    ReplaceChar(&f_ProjectRootDirectory, '\\', '/'); //XXX: used to relace stupid windows shit

    return f_ProjectRootDirectory;
}

int 
    main(int fp_ArgCount, char* fp_ArgVector[])
{
    signal(SIGSEGV, SegFaultHandler); //XXX: used for trying to close and flush logs on seg fault

    //Enable ANSI colour codes for windows console grumble grumble
#if defined(PEACH_PLATFORM_WINDOWS) && defined(PEACH_USING_OS_TERMINAL)
    BongoJam::EnableWindowsConsoleColours();
#endif

    std::string f_ProjectRootDirectory = GetProjectRootDirectory(fp_ArgVector[0]);

    try
    {
        //these boys are < 1kb so stack is fine for a CLI tool owo
        BongoJam::CLI bongo_cli;
        BongoJam::CompilerThreadPool<1> f_CompilerThreadPool; //TODO: this should come after arg parse to figure out -J flags for how many jobs we can do ^w^

        //int result = bongo_manager->ParseArguments(fp_ArgCount, fp_ArgVector);

        int result = bongo_cli.RunTest(f_ProjectRootDirectory + "/tests/Variables/main.bj", f_CompilerThreadPool);

        if (result != BONGO_OK)
        {
            return result;
        }
    }
    catch (const std::exception& Exception) ///Try to ensure all destructors are called especially close() on LogManager
    {
        BONGO_PRINT_ERROR_FMT("Unhandled exception: {}", Exception.what());
        return -69; //hehe Xd
    }

    return EXIT_SUCCESS;
}