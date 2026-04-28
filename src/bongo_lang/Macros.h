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

#include <stdio.h>
#include <source_location>
#include <cstdlib>

#include <fmt/format.h>

//================================================================================ Force Inline ================================================================================//

#if defined(_MSC_VER)
#   define BONGO_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#   define BONGO_FORCEINLINE inline __attribute__((always_inline))
#else
#   define BONGO_FORCEINLINE inline // Fallback for anything else
#endif /*BONGO_FORCEINLINE*/

//================================================================================ Printing + Colours ================================================================================//

// ANSI colour codes
#define BONGO_COL_RESET            "\033[0m"

#define BONGO_COL_RED              "\x1B[31m"
#define BONGO_COL_GREEN            "\x1B[32m"
#define BONGO_COL_YELLOW           "\x1B[33m"
#define BONGO_COL_BLUE             "\x1B[34m"
#define BONGO_COL_MAGENTA          "\x1B[35m"
#define BONGO_COL_CYAN             "\x1B[36m"
#define BONGO_COL_WHITE            "\x1B[37m"

#define BONGO_COL_BRIGHT_RED       "\x1B[91m"
#define BONGO_COL_BRIGHT_GREEN     "\x1B[92m"
#define BONGO_COL_BRIGHT_YELLOW    "\x1B[93m"
#define BONGO_COL_BRIGHT_BLUE      "\x1B[94m"
#define BONGO_COL_BRIGHT_MAGENTA   "\x1B[95m"
#define BONGO_COL_BRIGHT_CYAN      "\x1B[96m"
#define BONGO_COL_BRIGHT_WHITE     "\x1B[97m"

// wrap a string literal in a colour, zero runtime cost, pure compile-time concat
#define BONGO_COLOURED(fp_Colour, fp_String) fp_Colour fp_String BONGO_COL_RESET

#ifdef BONGO_DEBUG
# define BONGO_ENABLE_TERMINAL
#endif /*BONGO_DEBUG*/

// print macros, gets thrown out for release builds owo
#ifdef BONGO_ENABLE_TERMINAL
#   define BONGO_PRINT(fp_String, fp_Colour) fmt::print(stdout, "{}{}{}\n", fp_Colour, fp_String, BONGO_COL_RESET)
#   define BONGO_PRINT_ERROR(fp_String) fmt::print(stderr, "{}{}{}\n", BONGO_COL_BRIGHT_RED, fp_String ,BONGO_COL_RESET)
#   define BONGO_PRINT_FMT(fp_Colour, fp_Format, ...) fmt::print(stdout, "{}" fp_Format "{}\n", fp_Colour, __VA_OPT__(__VA_ARGS__,) BONGO_COL_RESET)
#   define BONGO_PRINT_ERROR_FMT(fp_Format, ...) fmt::print(stdout, "{}" fp_Format "{}\n", BONGO_COL_BRIGHT_RED, __VA_OPT__(__VA_ARGS__,) BONGO_COL_RESET)

#else
#   define BONGO_PRINT(fp_String, fp_Colour)                   ((void)0)
#   define BONGO_PRINT_ERROR(fp_String)                        ((void)0)
#   define BONGO_PRINT_FMT(fp_Colour, fp_Format, ...)          ((void)0)
#   define BONGO_PRINT_ERROR_FMT(fp_Colour, fp_Format, ...)    ((void)0)
#endif /*BONGO_ENABLE_TERMINAL*/


//================================================================================ Likely/Unlikely UwU ================================================================================//

#if defined(__GNUC__) || defined(__clang__)
#   define BONGO_LIKELY(fp_Condition)   __builtin_expect(!!(fp_Condition), 1)
#   define BONGO_UNLIKELY(fp_Condition) __builtin_expect(!!(fp_Condition), 0)
#else
#   define BONGO_LIKELY(fp_Condition)   (fp_Condition) //just use PGO or whatever for MSVC owo
#   define BONGO_UNLIKELY(fp_Condition) (fp_Condition)
#endif

//================================================================================ Assert ================================================================================//

namespace BongoJam{
    [[noreturn]] BONGO_FORCEINLINE void
        AssertFail
        (
            const char* fp_Condition,
            const std::source_location& fp_Loc = std::source_location::current()
        ) 
        noexcept
    {
        fprintf
        (
            stderr,
            BONGO_COL_BRIGHT_RED
            "\n[BONGO ASSERT FAILED]\n"
            "  condition : %s\n"
            "  file      : %s\n"
            "  line      : %u\n"
            "  func      : %s\n"
            BONGO_COL_RESET "\n",
            fp_Condition, fp_Loc.file_name(), fp_Loc.line(), fp_Loc.function_name()
        );

        std::abort();
    }
}

#define BONGO_PANIC_IF(fp_Condition) (BONGO_LIKELY(fp_Condition) ? (void)0 : ::BongoJam::AssertFail(#fp_Condition))

#ifdef BONGO_DEBUG
# define BONGO_DEBUG_ASSERT(fp_Condition) BONGO_PANIC_IF(fp_Condition)
#else
# define BONGO_DEBUG_ASSERT(fp_Condition) ((void)0)
#endif /*BONGO_DEBUG_ASSERT*/
