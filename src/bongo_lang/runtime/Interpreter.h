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

#define BONGO_RUNTIME_VERSION "0.0.1"

#include "BumpAllocator.h"
#include "DynamicMemoryArena.h"

#include "GarbageCollector.h"

///STL
#include <unordered_map>
#include <functional>
#include <cmath>

#define BONGO_INT int64_t
#define BONGO_UINT uint64_t
#define BONGO_BOOL int64_t
#define BONGO_FLOAT double

constexpr const uint32_t MAX_STACK_SIZE = 8192;

namespace BongoJam {

    enum class ValueType : uint8_t
    {
        U8, U16, U32, U64, I8, I16, I32, I64, F32, F64, BOOL, STRING, CLASS_REF, STRUCT_REF, INVALID
    };

    struct Value
    {
        ValueType Type = ValueType::INVALID;

        union
        {
            int64_t i64;
            uint64_t u64;
            double dbl;
            void* ref;
            uint64_t raw;
        } val;

        explicit Value(ValueType fp_Type, int64_t fp_Value) : Type(fp_Type) { val.i64 = fp_Value; }
        explicit Value(ValueType fp_Type, uint64_t   fp_Value) : Type(fp_Type) { val.u64 = fp_Value; }
        explicit Value(ValueType fp_Type, double fp_Value) : Type(fp_Type) { val.dbl = fp_Value; }
        explicit Value(ValueType fp_Type, void* fp_Value) : Type(fp_Type) { val.ref = fp_Value; }

        Value() = default;

        void 
            DebugPrintOut() 
            const 
        {
            switch (Type)
            {
                case ValueType::I64:      printf("int: %d\n", val.i64); break;
                case ValueType::U64:      printf("float: %f\n", val.u64); break;
                case ValueType::F64:     printf("bool: %s\n", val.dbl ? "true" : "false"); break;
                case ValueType::STRING:   printf("string ptr: %p\n", val.ref); break;
                case ValueType::CLASS_REF:printf("class ref: %p\n", val.ref); break;
                case ValueType::STRUCT_REF:printf("struct ref: %p\n", val.ref); break;
                default: printf("invalid or uninitialized value\n");
            }
        }
    };

    struct CallFrame
    {
        //Function* FunctionPtr;
        size_t ReturnIP;
        size_t StackBase; // where this frame starts in m_Stack
    };

    enum HeapTag { STRING, LIST, DICTIONARY, ARRAY, CLASS, STRUCT, INVALID };

    struct HeapObject
    {
        unordered_map<string, Value> Fields; // class/struct members

        HeapTag Tag = HeapTag::INVALID;
        uint32_t Generation = 0;

        union
        {
            string* StringPtr; //strings uwu
            Value* ArrayPtr[2]; //heap allocated static lists are important for multi threaded access, since array's wont invalidate iterators upon resize since it doesn't resize uwu
            vector<Value>* ListPtr; //vector, so bj list's are guaranteed contiguous blocks (in virutal memory >w<)
            unordered_map<Value, Value>* DictionaryPtr; //hash map, don't require strong ordering of types just matches values since dictionaries don't have begin() and end() iterators so ye
            void* TypePtr; //classes/structs
        } u;
    };
}

namespace BongoJam {

    struct RuntimeSymbol 
    {
        string Name;
        ValueType Type;
        size_t StackOffset;
    };

    class
        BongoJamInterpreter
    {
    public:
        using NATIVE_FUNCTION = function<Value(BongoJamInterpreter&)>;

    //////////////////////////////////////////////
    // Private Class Members
    //////////////////////////////////////////////
    private:
        unordered_map<string, RuntimeSymbol> LocalSymbols;

        unique_ptr<Logger> runtime_logger = nullptr;

        BumpAllocator Stack = BumpAllocator(MAX_STACK_SIZE * sizeof(Value));
        vector<CallFrame> CallStack;

        DynamicMemoryArena<HeapObject> HeapStorage;

        Value* m_StackStart = static_cast<Value*>(Stack.Allocate(MAX_STACK_SIZE * sizeof(Value), alignof(Value)));

        size_t STACK_POINTER = 0;
        size_t PROGRAM_COUNTER = 0;
        size_t BASE_POINTER = 0;

        bool STATUS_REGISTER = false;

        //int8_t ZERO_FLAG = 0;

        unordered_map<string, NATIVE_FUNCTION> NativeFunctions;

        vector<string> ListOfDecodedStrings;

    public:
        BongoJamInterpreter();
        ~BongoJamInterpreter() = default;

    private:
        //////////////////////////////////////////////
        // Decoding Functions
        //////////////////////////////////////////////

        //void
        //    DecodeAndStoreUTF8Strings(vector<uint8_t>* fp_ByteCode);

        //////////////////////////////////////////////
        // Utility Functions
        //////////////////////////////////////////////

        void
            Push(Value fp_Value);

        Value
            Pop();

        void 
            PushStackFrame(size_t returnIP, size_t localCount);

        void 
            PopStackFrame();

    public:
        void 
            DumpStack()
        {
            printf("===== STACK DUMP =====\n");
            for (size_t i = 0; i < STACK_POINTER; ++i)
            {
                printf("[%zu] ", i);
                m_StackStart[i].DebugPrintOut();
            }
        }

        int64_t
            BongoTime(const string& fp_BongoScriptName);
    };
}