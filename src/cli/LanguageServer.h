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
#ifndef BONGO_SERVER_C_H_
#define BONGO_SERVER_C_H_

#include <stdint.h>

#ifdef __cplusplus

    //XXX: probably better to just pass bongomanager funcs using a C calling convention since its args are compatible and don't return anything just
    // writes artifacts to build output dir

    #if (defined(_WIN32) || defined(_WIN64)) && defined(BONGO_SHARED)
        #define BONGO_API extern "C" __declspec(dllexport)
    #else
        #define BONGO_API extern "C"
    #endif

#else //being used from C
    #define BONGO_API //leave empty when included in C file

#endif //C++ detection

typedef int64_t BONGO_ERROR_CODE;

BONGO_API BONGO_ERROR_CODE
    BONGO_ParseArguments(int fp_ArgCount, char* fp_ArgVector[]);

BONGO_API BONGO_ERROR_CODE
    BONGO_RegisterFunction();

BONGO_API BONGO_ERROR_CODE
    BONGO_CompileProject();


#endif //header guard uwu

