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
#include "LanguageServer.h"
#include "BongoManager.h"
#include <ErrorCodes.h>

#define BONGO_ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

template<size_t pm_MaximumAllowedThreads>
struct BongoContext
{

};

namespace BJ = BongoJam;

BONGO_API BONGO_ERROR_CODE
    BONGO_ParseArguments(int fp_ArgCount, char* fp_ArgVector[])
{
    int64_t result = 0;

    return BONGO_OK;
}

BONGO_API BONGO_ERROR_CODE
    BONGO_RegisterFunction()
{

    return BONGO_OK;
}

BONGO_API BONGO_ERROR_CODE
    BONGO_CompileProject()
{

    return BONGO_OK;
}