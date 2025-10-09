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
#define BONGO_USING_TERMINAL
#define BONGO_DEBUG

#include "../../include/cli/BongoManager.h"

#include <csignal>

constexpr const int FATAL_SEGMENTATION_FAULT = -6969;

static void
    SegFaultHandler(int fp_Signal) //primitive segfault handler
{
    BongoJam::PrintError(std::format("[!]FATAL SEGMENTATION FAULT: Crash signal received {}", fp_Signal));
    // possibly notify watchdog or dump stack trace
    exit(FATAL_SEGMENTATION_FAULT); //clean exit so everything calls their destructors
}

int 
    main(int fp_ArgCount, char* fp_ArgVector[])
{
    signal(SIGSEGV, SegFaultHandler); //XXX: used for trying to close and flush logs on seg fault

    //Enable ANSI colour codes for windows console grumble grumble
    #if defined(_WIN32) || defined(_WIN64)
        BongoJam::EnableColors();
    #endif

    try
    {
        std::unique_ptr<BongoJam::BongoManager<1>> bongo_manager = std::make_unique<BongoJam::BongoManager<1>>();

        //int result = bongo_manager->ParseArguments(fp_ArgCount, fp_ArgVector);

        int result = bongo_manager->RunTest("D:/Game Development/All Things Bongo/BongoJam/tests/Variables/main.bj");

        if (result != BongoJam::BONGO_OK)
        {
            return result;
        }
    }
    catch (const std::exception& Exception) ///Try to ensure all destructors are called especially close() on LogManager
    {
        BongoJam::PrintError(std::format("Unhandled exception: {}", Exception.what()));

        return -69;
    }

    return EXIT_SUCCESS;
}