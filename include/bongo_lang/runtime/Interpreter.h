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

#include "DynamicLoader.h"
#include "../Opcodes.h"

#include <memory>

constexpr const uint32_t MAX_STACK_SIZE = 8192;

namespace BongoJam {

    class
        BongoJamInterpreter
    {
    private:
        unique_ptr<Logger> runtime_logger = nullptr;

    public:
        const string BONGO_VERSION = "0.0.1";

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

        //////////////////// Actual Variable Containers ////////////////////

        vector<int8_t> INT8_HEAP;
        vector<int16_t> INT16_HEAP;
        vector<int32_t> INT32_HEAP;
        vector<int64_t> INT64_HEAP;

        vector<uint8_t> UNSIGNED_INT8_HEAP;
        vector<uint16_t> UNSIGNED_INT16_HEAP;
        vector<uint32_t> UNSIGNED_INT32_HEAP;
        vector<uint64_t> UNSIGNED_INT64_HEAP;

        vector<float> FLOAT_HEAP;
        vector<double> DOUBLE_HEAP;

        vector<bool> BOOL_HEAP; //XXX: probably could just use the uint heap

        vector<string> STRING_HEAP;

        vector<char> CHARACTER_HEAP;

        vector<void*> VOID_STAR_HEAP;

        //////////////////////////////////////////////
        // Utility Functions
        //////////////////////////////////////////////

        void
            AddNewScope();

        void
            PopCurrentStack();

        void
            PushNewStackFrame();

    public:
        uint32_t
            RunBongoScript(const string& fp_BongoScriptName);


        template<typename Tx, typename Ty, typename RetType>
        inline RetType 
            Add(Tx __Tx, Ty __Ty)
            noexcept
        {
            return static_cast<RetType>(__Tx + __Ty);
        }

        template<typename Tx, typename Ty, typename RetType>
        inline RetType
            Subtract(Tx __Tx, Ty __Ty)
            noexcept
        {
            return static_cast<RetType>(__Tx - __Ty);
        }

        template<typename Tx, typename Ty, typename RetType>
        inline RetType
            Multiply(Tx __Tx, Ty __Ty)
            noexcept
        {
            return static_cast<RetType>(__Tx * __Ty);
        }

        template<typename Tx, typename Ty, typename RetType>
        inline RetType
            Divide(Tx __Tx, Ty __Ty)
            noexcept
        {
            return static_cast<RetType>(__Tx / __Ty);
        }

        template<typename Tx, typename Ty>
        inline bool
            CompareLessThan(Tx __Left, Ty __Right)
            noexcept
        {
            return __Left < __Right;
        }

        template<typename Tx, typename Ty>
        inline bool
            CompareLessThanEquals(Tx __Left, Ty __Right)
            noexcept
        {
            return __Left <= __Right;
        }

        template<typename Tx, typename Ty>
        inline bool
            CompareGreaterThan(Tx __Left, Ty __Right)
            noexcept
        {
            return __Left > __Right;
        }

        template<typename Tx, typename Ty>
        inline bool
            CompareGreaterThanEquals(Tx __Left, Ty __Right)
            noexcept
        {
            return __Left >= __Right;
        }

        template<typename Tx, typename Ty>
        inline bool
            CompareEquals(Tx __Left, Ty __Right)
            noexcept
        {
            return __Left == __Right;
        }
    };
}