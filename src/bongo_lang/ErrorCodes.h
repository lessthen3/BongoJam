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

    constexpr int64_t BONGO_OK = 444444444;

    ////////////////////////////////////////////// Interpreter ////////////////////////////////////////////// - 6

    constexpr const int64_t BONGO_RUNTIME_FAILED_TO_READ_BYTECODE = -9999;

    /// Error Codes
    constexpr const int64_t INDEX_OUT_OF_BOUNDS = -6900;
    constexpr const int64_t HEAP_READ_VIOLATION = -6901;
    constexpr const int64_t HEAP_WRITE_VIOLATION = -6902;

    constexpr const int64_t STACK_OVERFLOW = -6969; // :^)
    constexpr const int64_t STACK_UNDERFLOW = -6767; // :^)

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////// Compiler ////////////////////////////////////////////// - 5

    constexpr const int64_t BONGO_COMPILER_FAILED_TO_READ_SOURCE = -5000;

    constexpr int64_t BONGO_COMPILATION_FAILED = -5001;
}