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
#pragma once

//#include <pybind11/pybind11.h>

//#include <Compiler.h>

//XXX: need to have C calling convention since this'll be called by external runtimes which probably just use a C calling convention esp C#
#if (defined(_WIN32) || defined(_WIN64)) && defined(BONGO_SHARED)
    #define BONGO_API extern "C" __declspec(dllexport)
#else
    #define BONGO_API extern "C"
#endif

namespace BongoLang {

    BONGO_API bool BONGO_Parse();

}
