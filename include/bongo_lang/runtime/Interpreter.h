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

///BongoJam
#include "DynamicLoader.h"
#include "../BongoGlue.h"

#include "BumpAllocator.h"
#include "DynamicMemoryArena.h"

#include "GarbageCollector.h"

///STL
#include <unordered_map>
#include <functional>

constexpr const uint32_t MAX_STACK_SIZE = 8192;

namespace BongoJam {
    const string BONGO_RUNTIME_VERSION = "0.0.1";
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
        typedef typename function<Value(BongoJamInterpreter&)> NATIVE_FUNCTION;

    private:
        unordered_map<string, RuntimeSymbol> LocalSymbols;

        unique_ptr<Logger> runtime_logger = nullptr;

        BumpAllocator Stack = BumpAllocator(MAX_STACK_SIZE * sizeof(Value));
        vector<CallFrame> CallStack;

        DynamicMemoryArena<HeapObject> HeapStorage;

        Value* m_StackStart = static_cast<Value*>(Stack.Allocate(MAX_STACK_SIZE * sizeof(Value), alignof(Value)));
        size_t m_StackTop = 0;


        unordered_map<string, NATIVE_FUNCTION> NativeFunctions;

    public:
        BongoJamInterpreter();
        ~BongoJamInterpreter() = default;

        //Enable ANSI colour codes for windows console grumble grumble
        #if (defined(_WIN32) || defined(_WIN64)) && defined(BONGO_USING_TERMINAL)
            bool
                EnableWindowsANSIColourCodes();
        #endif

    private:
        //////////////////////////////////////////////
        // Read Bongo Code
        //////////////////////////////////////////////

        bool
            ReadBytecodeFromFile
            (
                const string& fp_CompiledBytecodeFilePath,
                vector<uint8_t>& fp_Bytecode
            );

        //////////////////////////////////////////////
        // Decoding Functions
        //////////////////////////////////////////////

        uint32_t
            Decode32BitInt(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset);

        string
            DecodeUTF8String(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset);


        float
            DecodeFloat(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset);

        char
            Decode32BitChar(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset);

        bool
            DecodeBool(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset);

        //////////////////////////////////////////////
        // Utility Functions
        //////////////////////////////////////////////

        void
            Encode32BitInt(vector<uint8_t>* fp_ByteCode, uint32_t fp_Int);

        void
            DecodeAndStoreUTF8Strings(vector<uint8_t>* fp_ByteCode);

        //////////////////////////////////////////////
        // Class Members
        //////////////////////////////////////////////

        vector<string> ListOfDecodedStrings;

        //////////////////////////////////////////////
        // Utility Functions
        //////////////////////////////////////////////

        void
            PushNewStackFrame();

        void
            PopCurrentStack();

        void
            Push(Value fp_Value);

        Value
            Pop();

        void 
            PushFrame(size_t returnIP, size_t localCount);

        void 
            PopFrame();


    public:
        uint32_t
            RunBongoScript(const string& fp_BongoScriptName);
    };
}