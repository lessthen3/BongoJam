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
#define BONGO_USING_OS_CONSOLE
#define BONGO_DEBUG

#include "BongoManager.h"

#include <csignal>

constexpr const int FATAL_SEGMENTATION_FAULT = -6969;

static void
    SegFaultHandler(int fp_Signal) //primitive segfault handler
{
    BongoJam::PrintError(std::format("[!]FATAL SEGMENTATION FAULT: Crash signal received {}", fp_Signal));
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
        BongoJam::PrintError("Failed to find the top-level directory 'Peach-E'!", BongoJam::Colours::Magenta);
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
    #if (defined(_WIN32) || defined(_WIN64)) && defined(BONGO_USING_OS_CONSOLE)
        BongoJam::EnableWindowsConsoleColours();
    #endif

    std::string f_ProjectRootDirectory = GetProjectRootDirectory(fp_ArgVector[0]);

    try
    {
        std::unique_ptr<BongoJam::BongoManager<1>> bongo_manager = std::make_unique<BongoJam::BongoManager<1>>();

        //int result = bongo_manager->ParseArguments(fp_ArgCount, fp_ArgVector);

        int result = bongo_manager->RunTest(f_ProjectRootDirectory + "/tests/Variables/main.bj");

        if (result != BongoJam::BONGO_OK)
        {
            return result;
        }
    }
    catch (const std::exception& Exception) ///Try to ensure all destructors are called especially close() on LogManager
    {
        BongoJam::PrintError(std::format("Unhandled exception: {}", Exception.what()));

        return -69; //hehe Xd
    }

    return EXIT_SUCCESS;
}