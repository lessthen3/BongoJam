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

///STL
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

namespace BongoJam
{
    using namespace std;

    //////////////////////////////////////////////
    // Opcodes Lookup for Translation
    //////////////////////////////////////////////

    enum BJ_OP : uint8_t //these opcodes are flags that indicate to the interpreter what information is going to follow it in the byte stream
    {
        NOP = 0x00,

        //////////////////// Basic Math Operations ////////////////////

        ADD,
        SUB,

        DIV,
        MULT,

        POW,
        SQRT,
        MOD,

        CEIL,
        FLOOR,

        AND,
        OR,
        XOR,
        NOT,

        SHL,
        SHR, //unsigned
        SAR, //signed

        DEC,
        INC,

        //////////////////// Memory Operations ////////////////////

        ENTER,
        LEAVE, //ye owo

        ///Stack
        PUSH, //stack allocates a var
        POP, //frees stack alloc'd var in interpreter
        STORE_LOCAL,
        LOAD_LOCAL,
        /// Heap
        HEAP_ALLOC, //used for creating heap allocations
        HEAP_FREE, //deWete oWo ><, frees a heap alloc
        STORE_GLOBAL, //stores a value at heap address
        LOAD_GLOBAL, //loads a global heap value

        COPY,
        MOVE, 

        //////////////////// Control Flow Operations ////////////////////

        JMP,
        JE,
        JNE,
        JNZ,

        CMP_SIGNED,
        CMP_UNSIGNED, //unsigned integer compar
        CMP_EQ, //==
        CMP_NE,  //!=
        CMP_LT, // <
        CMP_GT, // >
        CMP_LE, // <=
        CMP_GE, // >=

        RET,
        LABEL,
        CALL,

        //////////////////// Extern Call for C/C++ ////////////////////

        NATIVE_CALL, //call into native code baked into runtime
        EXTERN_CALL, //call to dynamically loaded C/C++

        //////////////////// Syscalls ////////////////////

        STDOUT,
        STDERR,
        STDIN,

        CLOCK_START,
        CLOCK_END,

        SLEEP,
        THREAD,

        //////////////////// Internal VM Tracking Codes ////////////////////

        ///Debugging
        LINE_NUMBER, //Used for tracking the exact line of code that threw a runtime error
        DEBUG_LINE,
        BREAKPOINT, 
        ///Reflection
        TYPE_TAG,
        THREAD_ID,

        //////////////////// Error Handling ////////////////////

        THROW,
        TRY,
        CATCH,
        EXCEPTION,

        //////////////////// Primitive Types ////////////////////

        INT_VALUE,
        UNSIGNED_INT_VALUE,

        FLOAT_VALUE,
        DOUBLE_VALUE,

        BOOL_VALUE, //false or true follows, 0 = false, 1 = true as always

        CHAR_VALUE,
        STRING_VALUE, //string literal value always follows, pattern is: str size in bytes -> encoded utf-8 str

        VOID_VALUE, //idk IF NEEDED

        //////////////////// Boolean Comparison Operations ////////////////////

        LOGICAL_AND,
        LOGICAL_OR,
        LOGICAL_NOT,

        //////////////////// Stop op UwU ////////////////////

        HALT = 0xFF //>O<
    };

    struct SSAInstruction
    {
        BJ_OP OP = BJ_OP::NOP;
        string DEST;
        vector<string> SRC;
    };

}//namespace BongoJam