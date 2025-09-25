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

namespace BongoJam
{
    using namespace std;

    enum class ValueType : uint8_t
    {
        I32, F32, HEAP_REF, INVALID
    };

    struct Value
    {
        ValueType Type;
        union
        {
            int32_t i32;
            float   f32;
            void* ref;
            uint64_t raw;
        } u;

        Value(ValueType fp_Type, int32_t fp_Value) : Type(fp_Type) { u.i32 = fp_Value; }
        Value(ValueType fp_Type, float   fp_Value) : Type(fp_Type) { u.f32 = fp_Value; }
        Value(ValueType fp_Type, void* fp_Value) : Type(fp_Type) { u.ref = fp_Value; }

        void
            DebugPrintOut()
        {
            //do smth idk
        }
    };

    struct CallFrame
    {
        //Function* FunctionPtr;
        size_t ReturnIP;
        size_t StackBase; // where this frame starts in m_Stack
    };

    enum HeapTag { STRING, ARRAY, STRUCT };

    struct HeapObject
    {
        unordered_map<string, Value> Fields; // class/struct members

        HeapTag Tag;
        uint32_t Generation;

        union
        {
            string* StringVal;
            vector<Value>* ArrayVal;
            void* StructPtr;
        } u;
    };

    //////////////////////////////////////////////
    // Opcodes Lookup for Translation
    //////////////////////////////////////////////

    enum OPCODES : uint8_t //these opcodes are flags that indicate to the interpreter what information is going to follow it in the byte stream
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

        //////////////////// Memory Operations ////////////////////
        PUSH_NEW_STACK_FRAME,
        POP_CURRENT_STACK_FRAME, //IDK

        ///Stack
        PUSH, //stack allocates a var
        POP, //frees stack alloc'd var in interpreter
        STORE_LOCAL,
        LOAD_LOCAL,
        /// Heap
        HEAP_ALLOC, //used for creating heap allocations
        HEAP_FREE, //delete baby, frees a heap alloc
        STORE_GLOBAL, //stores a value at heap address
        LOAD_GLOBAL, //loads a global heap value

        COPY,
        MOVE,

        //////////////////// Control Flow Operations ////////////////////

        JUMP,
        JUMP_IF,
        JUMP_NZ,
        JUMP_NE,

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

        VOID_VALUE,

        //////////////////// Boolean Comparison Operations ////////////////////

        LOGICAL_AND,
        LOGICAL_OR,
        LOGICAL_NOT,

        //////////////////// Function/Method Operations ////////////////////

        FUNC_ENTER,
        FUNC_LEAVE,

        LOAD_ARG,
        STORE_ARG,

        //////////////////// Stop op UwU ////////////////////

        HALT = 0xFF //>O<
    };

    struct SSAInstruction
    {
        OPCODES OP = OPCODES::NOP;
        string DEST;
        vector<string> SRC;
    };

}//namespace BongoJam