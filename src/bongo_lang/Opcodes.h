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
#include <cstdint>

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

        ADD_I, //abelian op, maps back to the original type no matter what
        ADD_F,
        ADD_U,

        /// ONLY INTS ///

        ADD_I_U,

        /// ONLY FLOATS ///  , stops implicit cast from float -> int let the user handle that it'll be the same time complexity whether i do it or they do it so give em the choice between floor or ceil uwu
        ADD_I_F,
        ADD_F_U,

        MULT_I, //abelian op, maps back to the original type no matter what
        MULT_F,
        MULT_U,
        
        /// ONLY FLOATS ///  , stops implicit cast from float -> int let the user handle that it'll be the same time complexity whether i do it or they do it so give em the choice between floor or ceil uwu
        MULT_I_F,
        MULT_I_U,
        MULT_F_U,
        /////////////////////

        ADD_S, //adding strings is non abelian >w<
        ADD_S_I,
        ADD_S_U,
        ADD_S_F,

        ADD_I_S,
        ADD_U_S,
        ADD_F_S,

        SUB_I, //:^) non abelian ops, always maps back to the original type no matter what
        SUB_F,
        SUB_U,

        /// ONLY INTS ///

        SUB_I_U,
        SUB_U_I,
        
        /// ONLY FLOATS ///  , stops implicit cast from float -> int let the user handle that it'll be the same time complexity whether i do it or they do it so give em the choice between floor or ceil uwu
        SUB_I_F,
        SUB_F_I,

        SUB_U_F,
        SUB_F_U,

        //////////////////// These ops will have to choose which type to cast to since these maps are not homomorphisms ////////////////////

        /// ONLY FLOATS ///  , stops implicit cast from float -> int let the user handle that it'll be the same time complexity whether i do it or they do it so give em the choice between floor or ceil uwu
        DIV_I, //assert denom != 0
        DIV_F,
        DIV_U,

        DIV_I_F,
        DIV_F_I,
        
        DIV_I_U,
        DIV_U_I,

        DIV_U_F,
        DIV_F_U,

        SQRT_F, //have to assert i >= 0
        SQRT_I, //have to assert i >= 0
        SQRT_U, //always >= 0 uwu

        POW_F, //F^F

        /// ONLY INT ///

        POW_I, //I^I can only be stored in an int64_t, prevents underflow on uint64_t
        POW_I_U,
        POW_U_I,

        /// ONLY UINT /// this will prevent integer overflow

        POW_U,

        //////////////////// maps uint -> uint ////////////////////

        MOD_I_U,
        MOD_U_I,
        MOD_I,
        MOD_U,

        CEIL, //only takes floats
        FLOOR,

        AND,
        OR,
        XOR,
        NOT,

        SHL,
        SHR, //unsigned
        SAR, //signed

        DEC_I,
        DEC_U,

        INC_I,
        INC_U,

        //////////////////// Memory Operations ////////////////////

        ENTER,
        LEAVE, //ye owo

        ///Stack //stack allocates a var
        PUSH_I, //bools are ints
        PUSH_U,

        PUSH_F,
        PUSH_S, 

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
        JZ,
        JNZ,

        RET,
        LABEL,
        CALL,

        //////////////////// Bool Operators UwU ////////////////////

        CMP_EQ_I, //== abelian op
        CMP_EQ_F,
        CMP_EQ_U,
        CMP_EQ_I_F,
        CMP_EQ_I_U,
        CMP_EQ_U_F,

        CMP_NE_I,  //!= abelian op
        CMP_NE_F,
        CMP_NE_U,

        CMP_NE_I_F,
        CMP_NE_I_U,
        CMP_NE_U_F,

        //Note: we only need one direction since U < F = F > U uwu , and if we wanna check 5 > 3 then we can just check 3 < 5 and invert the cond uwu
        CMP_LT_I, //a < b
        CMP_LT_F,
        CMP_LT_U,

        CMP_LT_I_F,
        CMP_LT_I_U,
        CMP_LT_U_F,

        CMP_GT_I_F, // a > b
        CMP_GT_I_U,
        CMP_GT_U_F,
        
        CMP_LE_I, // a <= b
        CMP_LE_F,
        CMP_LE_U,

        CMP_LE_I_F,
        CMP_LE_I_U,
        CMP_LE_U_F,

        CMP_GE_I_F, // a >= b
        CMP_GE_I_U,
        CMP_GE_U_F,

        //////////////////// Type Recasts ////////////////////

        CAST_I_TO_F,
        CAST_U_TO_F,

        CAST_F_TO_U,
        CAST_F_TO_I,

        CAST_I_TO_U,
        CAST_U_TO_I,

        //////////////////// C/C++ Function Calls ////////////////////

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
        VARIABLE_ID, //idk if u wanna check if(var_name("myVar")) ... can just reduce to an id and verify at compile time

        //////////////////// Error Handling ////////////////////

        THROW,
        TRY,
        CATCH,
        EXCEPTION,

        //////////////////// Boolean Comparison Operations ////////////////////

        LOGICAL_AND,
        LOGICAL_OR,
        LOGICAL_NOT,

        //////////////////// Stop op UwU ////////////////////

        HALT //>O<
    };

    struct SSAInstruction
    {
        BJ_OP OP = BJ_OP::NOP;
        //string DEST;
        //vector<string> SRC;
    };

}//namespace BongoJam