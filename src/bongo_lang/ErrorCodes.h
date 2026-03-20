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

    constexpr int64_t BONGO_OK = 69;

    ////////////////////////////////////////////// Internal ////////////////////////////////////////////// - 1

    constexpr int64_t TRIED_TO_PASS_NULLPTR_REF_TO_COMPILATION_UNIT = -1000;
    constexpr int64_t UNABLE_TO_START_BATCH_COMPILATION = -1001;

    constexpr int64_t BONGO_NO_MAIN_FOUND = -10100;
    constexpr int64_t BONGO_MULTIPLE_MAINS_FOUND = -10101;

    constexpr int64_t NO_ARGUMENT_PROVIDED = -1101;
    constexpr int64_t UNKNOWN_OR_INCOMPLETE_ARGUMENT = -1102;
    constexpr int64_t INVALID_SET_ARGUMENT = -1103;
    constexpr int64_t INVALID_SCRIPT_TARGET = -1104;
    constexpr int64_t SCRIPT_DOES_NOT_EXIST = -1105;

    constexpr int64_t NO_SCRIPTS_GIVEN = -1106;
    constexpr int64_t NO_OUTPUT_FILE_NAME_GIVEN = -1107;

    constexpr int64_t BONGO_FAILED_TO_LOAD_PROJECT = -1108;

    constexpr int64_t BONGO_NO_BUILD_TYPE_SPECIFIED = -1109;
    constexpr int64_t BONGO_NO_OUTPUT_TYPE_SPECIFIED = -1110;

    ////////////////////////////////////////////// Interpreter ////////////////////////////////////////////// - 6

    constexpr  int64_t BONGO_RUNTIME_FAILED_TO_READ_BYTECODE = -9999;

    /// Error Codes
    constexpr  int64_t INDEX_OUT_OF_BOUNDS = -6900;
    constexpr  int64_t HEAP_READ_VIOLATION = -6901;
    constexpr  int64_t HEAP_WRITE_VIOLATION = -6902;

    constexpr  int64_t STACK_OVERFLOW = -6969; // :^)
    constexpr  int64_t STACK_UNDERFLOW = -6767; // :^)

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////// Compiler ////////////////////////////////////////////// - 5

    constexpr  int64_t BONGO_COMPILER_FAILED_TO_READ_SOURCE = -5000;

    constexpr int64_t BONGO_COMPILATION_FAILED = -5001;

    ////////////////////////////////////////////// Lexer ////////////////////////////////////////////// - 3

    constexpr int64_t BONGO_FAILED_TO_LEX_SCRIPT = -3000;
}