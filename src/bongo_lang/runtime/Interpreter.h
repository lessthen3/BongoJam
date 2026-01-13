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

///BongoJam
#include "DynamicLoader.h"
#include "../Opcodes.h"
#include "../Serializer.h"

#include "BumpAllocator.h"
#include "DynamicMemoryArena.h"

#include "GarbageCollector.h"

///STL
#include <unordered_map>
#include <functional>
#include <cmath>

constexpr const uint32_t MAX_STACK_SIZE = 8192;

namespace BongoJam {

    enum class ValueType : uint8_t
    {
        I32, I64, F32, F64, BOOL, STRING, CLASS_REF, STRUCT_REF, INVALID
    };

    struct Value
    {
        ValueType Type = ValueType::INVALID;

        union
        {
            int32_t i32;
            int64_t i64;
            float   f32;
            double f64;
            void* ref;
            uint64_t raw;
            bool boolean;
        } u;

        explicit Value(ValueType fp_Type, int32_t fp_Value) : Type(fp_Type) { u.i32 = fp_Value; }
        explicit Value(ValueType fp_Type, float   fp_Value) : Type(fp_Type) { u.f32 = fp_Value; }
        explicit Value(ValueType fp_Type, double fp_Value) : Type(fp_Type) { u.f64 = fp_Value; }
        explicit Value(ValueType fp_Type, void* fp_Value) : Type(fp_Type) { u.ref = fp_Value; }

        Value() = default;

        // Primitives
        static Value FromInt(int32_t v) { Value val; val.Type = ValueType::I32;   val.u.i32 = v; return val; }
        static Value FromFloat(float v) { Value val; val.Type = ValueType::F32;   val.u.f32 = v; return val; }
        static Value FromBool(bool v) { Value val; val.Type = ValueType::BOOL;  val.u.boolean = v; return val; }

        // Heap Refs
        static Value FromRef(void* ref, ValueType type)
        {
            Value val;
            val.Type = type;
            val.u.ref = ref;
            return val;
        }

        void 
            DebugPrintOut() 
            const 
        {
            switch (Type)
            {
                case ValueType::I32:      printf("int: %d\n", u.i32); break;
                case ValueType::F32:      printf("float: %f\n", u.f32); break;
                case ValueType::BOOL:     printf("bool: %s\n", u.boolean ? "true" : "false"); break;
                case ValueType::STRING:   printf("string ptr: %p\n", u.ref); break;
                case ValueType::CLASS_REF:printf("class ref: %p\n", u.ref); break;
                case ValueType::STRUCT_REF:printf("struct ref: %p\n", u.ref); break;
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

    enum HeapTag { STRING, LIST, DICTIONARY, ARRAY, TYPE, INVALID };

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

        bool STATUS_REGISTER = false;

        unordered_map<string, NATIVE_FUNCTION> NativeFunctions;

        vector<string> ListOfDecodedStrings;

    public:
        BongoJamInterpreter();
        ~BongoJamInterpreter() = default;

    private:
        //////////////////////////////////////////////
        // Decoding Functions
        //////////////////////////////////////////////

        void
            DecodeAndStoreUTF8Strings(vector<uint8_t>* fp_ByteCode);

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