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

#include <cstdint>

namespace BongoJam{

    constexpr int BONGO_OK = 444444444;

    ////////////////////////////////////////////// Interpreter ////////////////////////////////////////////// - 6

    constexpr const int32_t BONGO_RUNTIME_FAILED_TO_READ_BYTECODE = -9999;

    /// Error Codes
    constexpr const int32_t INDEX_OUT_OF_BOUNDS = -6900;
    constexpr const int32_t HEAP_READ_VIOLATION = -6901;
    constexpr const int32_t HEAP_WRITE_VIOLATION = -6902;

    constexpr const int32_t STACK_OVERFLOW = -6969; // :^)

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////// Compiler ////////////////////////////////////////////// - 5

    constexpr const int32_t BONGO_COMPILER_FAILED_TO_READ_SOURCE = -5000;

    constexpr int32_t BONGO_COMPILATION_FAILED = -5001;
}