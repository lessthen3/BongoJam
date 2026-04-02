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
#ifndef BONGO_ERROR_CODES_API_C_H_
#define BONGO_ERROR_CODES_API_C_H_

#include <stdint.h>

#ifdef __cplusplus
    #define BONGO_CONSTEXPR constexpr
#else
    #define BONGO_CONSTEXPR
#endif

BONGO_CONSTEXPR int32_t BONGO_OK = 69;

BONGO_CONSTEXPR int32_t BONGO_NO_SOURCE_FILES_IN_PROJECT_AT_COMPILE_REQUEST = -13000;
BONGO_CONSTEXPR int32_t BONGO_NO_MAIN_FOUND_IN_PROJECT_AT_COMPILE_REQUEST = -13001;


////////////////////////////////////////////// Internal ////////////////////////////////////////////// - 1

BONGO_CONSTEXPR int32_t TRIED_TO_PASS_NULLPTR_REF_TO_COMPILATION_UNIT = -1000;
BONGO_CONSTEXPR int32_t UNABLE_TO_START_BATCH_COMPILATION = -1001;

BONGO_CONSTEXPR int32_t BONGO_NO_MAIN_FOUND = -10100;
BONGO_CONSTEXPR int32_t BONGO_MULTIPLE_MAINS_FOUND = -10101;

BONGO_CONSTEXPR int32_t NO_ARGUMENT_PROVIDED = -1101;
BONGO_CONSTEXPR int32_t UNKNOWN_OR_INCOMPLETE_ARGUMENT = -1102;
BONGO_CONSTEXPR int32_t INVALID_SET_ARGUMENT = -1103;
BONGO_CONSTEXPR int32_t INVALID_SCRIPT_TARGET = -1104;
BONGO_CONSTEXPR int32_t SCRIPT_DOES_NOT_EXIST = -1105;

BONGO_CONSTEXPR int32_t NO_SCRIPTS_GIVEN = -1106;
BONGO_CONSTEXPR int32_t NO_OUTPUT_FILE_NAME_GIVEN = -1107;

BONGO_CONSTEXPR int32_t BONGO_FAILED_TO_LOAD_PROJECT = -1108;

BONGO_CONSTEXPR int32_t BONGO_NO_BUILD_TYPE_SPECIFIED = -1109;
BONGO_CONSTEXPR int32_t BONGO_NO_OUTPUT_TYPE_SPECIFIED = -1110;

////////////////////////////////////////////// Interpreter ////////////////////////////////////////////// - 6

BONGO_CONSTEXPR  int32_t BONGO_RUNTIME_FAILED_TO_READ_BYTECODE = -9999;

/// Error Codes
BONGO_CONSTEXPR  int32_t INDEX_OUT_OF_BOUNDS = -6900;
BONGO_CONSTEXPR  int32_t HEAP_READ_VIOLATION = -6901;
BONGO_CONSTEXPR  int32_t HEAP_WRITE_VIOLATION = -6902;

BONGO_CONSTEXPR  int32_t STACK_OVERFLOW = -6969; // :^)
BONGO_CONSTEXPR  int32_t STACK_UNDERFLOW = -6767; // :^)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////// Compiler ////////////////////////////////////////////// - 5

BONGO_CONSTEXPR  int32_t BONGO_COMPILER_FAILED_TO_READ_SOURCE = -5000;

BONGO_CONSTEXPR int32_t BONGO_COMPILATION_FAILED = -5001;

////////////////////////////////////////////// Lexer ////////////////////////////////////////////// - 3

BONGO_CONSTEXPR int32_t BONGO_FAILED_TO_LEX_SCRIPT = -3000;

#endif /*BONGO_ERROR_CODES_API_C_H_*/
