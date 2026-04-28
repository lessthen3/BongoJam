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
#define BONGO_USING_CONSOLE

#include "Interpreter.h"

///BongoJam
#include "../ErrorCodes.h"
#include "../DynamicLoader.h"
#include "../Opcodes.h"
#include "../BinaryCodec.h"
#include "../FileIO.h"

///STL
#include <iostream>

namespace BongoJam {

    BongoJamInterpreter::BongoJamInterpreter()
    {
        runtime_logger = Logger::CreateUnique("BongoJamInterpreter", PEACH_LOGGER_DEFAULT_FLAGS, PEACH_LOGGER_DEFAULT_OUTPUT_DIR);

        if (not runtime_logger)
        {
            throw runtime_error("WTF MANG LOGGER FAILED TO INITIALIZE FROM BongoLinker WTF MANG");
        }

        runtime_logger->Info("BongoJamInterpreter Logger intialized properly!", "BongoJamInterpreter");
    }

    void 
        BongoJamInterpreter::Push(Value fp_Value)
    {
        if (STACK_POINTER >= MAX_STACK_SIZE)
        {
            throw overflow_error("Stack overflow");
        }

        m_StackStart[STACK_POINTER++] = fp_Value;
    }

    Value 
        BongoJamInterpreter::Pop()
    {
        if (STACK_POINTER == 0)
        {
            throw underflow_error("Stack underflow");
        }

        return m_StackStart[--STACK_POINTER];
    }

    void
        BongoJamInterpreter::PushStackFrame
        (
            size_t fp_ReturnAddress, 
            size_t fp_LocalCount
        )
    {
        // Reserve space for locals
        if (STACK_POINTER + fp_LocalCount >= MAX_STACK_SIZE)
        {
            throw runtime_error("Stack overflow in PushFrame");
        }

        CallStack.emplace_back(fp_ReturnAddress, STACK_POINTER);

        BASE_POINTER = STACK_POINTER;
        STACK_POINTER += fp_LocalCount;
    }

    void 
        BongoJamInterpreter::PopStackFrame()
    {
        if (CallStack.empty())
        {
            throw runtime_error("CallStack underflow in PopFrame");
        }

        STACK_POINTER = CallStack.back().StackBase; // Rewind stack to previous base

        CallStack.pop_back();
    }


    //////////////////////////////////////////////
    // Decoding Functions
    //////////////////////////////////////////////
    /*
    void
        BongoJamInterpreter::DecodeAndStoreUTF8Strings(vector<uint8_t>* fp_ByteCode)
    {
        size_t _p = 0;

        for (_p; _p < fp_ByteCode->size(); _p++)
        {
            if ((*fp_ByteCode)[_p] == STRING_VALUE) //bongo-code for a String literal
            {
                _p++; //advance one to look for string size

                size_t f_InitialIndex = _p; //start index of actual string index bytes

                //push the actual string put together into a vector
                ListOfDecodedStrings.push_back
                (
                    BinaryCodec::DecodeStringUTF8<uint32_t>(*fp_ByteCode, _p)
                );

                //encode the uint32_t that represents the index of the string
                vector<uint8_t> f_IndexBytes;
                BinaryCodec::EncodeUint32(f_IndexBytes, ListOfDecodedStrings.size() - 1);

                //erase the encoded string bytes, _p should be sitting on the last byte of the encoded string
                fp_ByteCode->erase(fp_ByteCode->begin() + f_InitialIndex, fp_ByteCode->begin() + _p + 1);

                //insert the uint32_t that represents the string index
                fp_ByteCode->insert(fp_ByteCode->begin() + f_InitialIndex, f_IndexBytes.begin(), f_IndexBytes.end());

                //move the program pointer back to the index of the string-index, then the for-loop will advance _p to the next byte
                _p = f_InitialIndex + 4; //4 bytes = uint32_t

                //should be sitting on the last byte of the uint32_t representing the vector index of the string, the for-loop should advance to the next byte
            }
        }
    }
    */
 
    //////////////////////////////////////////////
    // Runtime Functions
    //////////////////////////////////////////////

    int64_t
        BongoJamInterpreter::BongoTime(const string& fp_BongoScriptName)
    {
        using BongoProgram = vector<uint8_t>;

        BongoProgram BONGO_PROGRAM;

        if (not FileIO::ReadBinaryIntoVector(fp_BongoScriptName, {".bongo"}, BONGO_PROGRAM, runtime_logger.get())) //stop execution immediately if the file was not able to be read
        {
            return BONGO_RUNTIME_FAILED_TO_READ_BYTECODE;
        }

        //DecodeAndStoreUTF8Strings(&BONGO_PROGRAM);
        
        //gcc rlly didnt like the const const const const owo
        const size_t f_Size = BONGO_PROGRAM.size(); //you never know if ones enough, gotta throw in a few more just in case

        size_t _l = 0; //line counter

        STACK_POINTER = 0; //in case the interpreter runs multiple programs during its lifetime uwu
        PROGRAM_COUNTER = 0;
        BASE_POINTER = 0;

        while(1)
        {
            switch (BONGO_PROGRAM[PROGRAM_COUNTER])
            {
            case PUSH_I:
            {
                Push(Value{ ValueType::I64, BinaryCodec::LittleEndian::DecodeNumber<int64_t>(BONGO_PROGRAM, ++PROGRAM_COUNTER) });
            }
            break;
            case PUSH_F:
            {
                Push(Value{ ValueType::F64, BinaryCodec::LittleEndian::DecodeNumber<double>(BONGO_PROGRAM, ++PROGRAM_COUNTER) });
            }
            break;
            case PUSH_U:
            {
                Push(Value{ ValueType::U64, BinaryCodec::LittleEndian::DecodeNumber<uint64_t>(BONGO_PROGRAM, ++PROGRAM_COUNTER) });
            }
            break;
            case PUSH_S:
            {
                Push(Value{ ValueType::STRING,  (void*) new string(BinaryCodec::LittleEndian::DecodeStringUTF8<BONGO_STRING_CHAR_MAX_LENGTH>(BONGO_PROGRAM, ++PROGRAM_COUNTER)) });
            }
            break;
            case POP:
            {

            }
            break;
            case ENTER:
            {
                PushStackFrame(PROGRAM_COUNTER, 1);
                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case LEAVE:
            {
                PopStackFrame();
                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case JMP:
            {
                //jump offset should figure out that its -1 the actual offset in bytecode since decode advances a byte uwu, but thats a compile time thing not a runtime thing
                int64_t sv_JmpOffset = BinaryCodec::LittleEndian::DecodeNumber<int64_t>(BONGO_PROGRAM, ++PROGRAM_COUNTER);

                PROGRAM_COUNTER += sv_JmpOffset; //offset is signed so can go backwards or forwards
            }
            break;
            case STORE_LOCAL: 
            {
                uint8_t slot = BinaryCodec::LittleEndian::DecodeNumber<uint32_t>(BONGO_PROGRAM, ++PROGRAM_COUNTER);
                Value val = Pop();
                size_t addr = BASE_POINTER + slot;

                if (addr >= MAX_STACK_SIZE)
                {
                    throw overflow_error("STACK OVERFLOW: too much memory was used during program runtime");
                }

                m_StackStart[addr] = val;
            }
            break;
            case LOAD_LOCAL:
            {
                uint8_t slot = BONGO_PROGRAM[++PROGRAM_COUNTER]; //inc first because sitting on LOAD_LOCAL atm uwu
                size_t addr = BASE_POINTER + slot;
                if (addr >= MAX_STACK_SIZE) 
                {
                    throw runtime_error("MEMORY VIOLATION ERROR: tried to access variable located outside of stack memory");
                }

                Push(m_StackStart[addr]);
            }
            break;
            case LINE_NUMBER: //new-line bongo-code
            {
                _l++;
                continue;
            }
            break;
            case CAST_I_TO_F: //new-line bongo-code
            {
                Value sv_TopVal = Pop();

                sv_TopVal.val.dbl = static_cast<double>(sv_TopVal.val.i64);
            }
            break;
            case CAST_U_TO_F: //new-line bongo-code
            {
            }
            break;
            case CAST_I_TO_U: //new-line bongo-code
            {
            }
            break;
            case CAST_F_TO_U: //new-line bongo-code
            {
            }
            break;
            case CAST_U_TO_I: //new-line bongo-code
            {
            }
            break;
            case CAST_F_TO_I: //new-line bongo-code
            {
            }
            break;
            case ADD_I: 
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::I64, a.val.i64 + b.val.i64 });

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case SUB_I:
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::I64, a.val.i64 - b.val.i64 });

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case MULT_I:
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::I64, a.val.i64 * b.val.i64 });

                PROGRAM_COUNTER++; //move to next instruction 
            }
                break;
            case DIV_I:
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::F64, static_cast<double>(a.val.i64 / b.val.i64) });

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_EQ_I:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 == b.val.i64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_EQ_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.dbl == b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            break;
            case CMP_EQ_U:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.u64 == b.val.u64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_NE_I:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 != b.val.i64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT_I:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 < b.val.i64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.dbl < b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT_U:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.u64 < b.val.u64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT_I_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 < b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT_I_U:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 < b.val.u64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT_U_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.u64 < b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_GT_I_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 > b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_GT_I_U:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 > b.val.u64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_GT_U_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.u64 > b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LE_I:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.i64 <= b.val.i64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LE_F:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.dbl <= b.val.dbl);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LE_U:
            {
                Value b = Pop();
                Value a = Pop();

                STATUS_REGISTER = static_cast<bool>(a.val.u64 <= b.val.u64);

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case AND:
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::U64, a.val.u64 & b.val.u64 });

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case OR:
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::U64, a.val.u64 | b.val.u64 });

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case XOR:
            {
                Value b = Pop();
                Value a = Pop();

                Push(Value{ ValueType::U64, a.val.u64 ^ b.val.u64 });

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case LABEL: //function call
            {
                PROGRAM_COUNTER++;

                //read the 8-bit integer that indicates how many func arguments there are (i highly doubt anyone will define a function with more lmao, ill make it a 16-bit if neccesary)
                uint8_t s_NumberOfFuncArgs = BONGO_PROGRAM[PROGRAM_COUNTER];

                //idk if this is cocher LMAO - it wasn't so i changed it lmao, apparently _n < _n + C is always true, where C > 0. who would've ever thought
                for (PROGRAM_COUNTER; PROGRAM_COUNTER < s_NumberOfFuncArgs; PROGRAM_COUNTER++) //cycle through function call arguments
                {
                    switch (BONGO_PROGRAM[PROGRAM_COUNTER])
                    {
                        //fuck it everything is passed by referece, no more guessing every function has side effects lmfao
                    }
                }

            }
            break;
            case STDOUT: //print function
            {
                //we're going to decode the utf8 string directly from the bytecode, however we should do a once-over and decode all function names for the lib versions of the compiled bytecode
                std::cout << BinaryCodec::LittleEndian::DecodeStringUTF8<BONGO_STRING_CHAR_MAX_LENGTH>(BONGO_PROGRAM, ++PROGRAM_COUNTER);
            }
            break;
            case STDIN:
            {
                string sv_InputString;
                cin >> sv_InputString;
                Push(Value{ ValueType::STRING, (void*) new string(sv_InputString)});

                PROGRAM_COUNTER++;
            }
            break;
            case STDERR:
            {
                //we're going to decode the utf8 string directly from the bytecode, however we should do a once-over and decode all function names for the lib versions of the compiled bytecode
                std::cerr << BinaryCodec::LittleEndian::DecodeStringUTF8<BONGO_STRING_CHAR_MAX_LENGTH>(BONGO_PROGRAM, ++PROGRAM_COUNTER);
            }
            break;
            case CEIL:  //IMPORTANT: static analysis will prevent invalid values being pushed onto the stack here, and we just treat any value here as a double then recast after ig
            {
                Value sv_Val = Pop();

                Push(Value{ ValueType::I64, static_cast<int64_t>(ceil(sv_Val.val.dbl)) });

                PROGRAM_COUNTER++;
            }
            break;
            case FLOOR: //IMPORTANT: static analysis will prevent invalid values being pushed onto the stack here, and we just treat any value here as a double then recast after ig
            {
                Value sv_Val = Pop();
                Push(Value{ ValueType::I64, static_cast<int64_t>(floor(sv_Val.val.dbl)) });

                PROGRAM_COUNTER++;
            }
            break;
            case NATIVE_CALL:
            {
                string sv_FuncName = BinaryCodec::LittleEndian::DecodeStringUTF8<BONGO_STRING_CHAR_MAX_LENGTH>(BONGO_PROGRAM, ++PROGRAM_COUNTER);

                if (NativeFunctions.contains(sv_FuncName))
                {
                    auto native = NativeFunctions[sv_FuncName];
                    Value result = native(*this);
                    Push(result);
                }
                else
                {
                    throw runtime_error(fmt::format("FATAL ERROR: Native function with name: '{}' not found", sv_FuncName));
                }

                //counter advances from decode call uwu
            }
            break;
            //XXX: Compiler should always pad a halt call w a exit code after
            case HALT: //XXX: used for exit() or abort() calls
            {
                int64_t f_ExitCode = BinaryCodec::LittleEndian::DecodeNumber<int64_t>(BONGO_PROGRAM, ++PROGRAM_COUNTER);
                cout << "\n\n"; //XXX: padding for exit msg and last print msg from user script
                BONGO_PRINT_FMT(BONGO_COL_BRIGHT_CYAN, "\nBongoJam program exited with code {}", f_ExitCode);
                return f_ExitCode; //SHOULD return number returned by bj script main func
            }
            break;
            default: //THROW ERROR
                runtime_logger->Error(fmt::format("INTERNAL RUNTIME ERROR: Error at Line Number: {}, Error at BYTE-CODE: {}", _l, BONGO_PROGRAM[PROGRAM_COUNTER]), "Interpreter");
                runtime_logger->Error("Something terrible happened while running the code, invalid bytecode was generated by the compiler (sorry not your fault I think LOL)", "Interpreter");
                runtime_logger->Debug("BongoJam program exited with code -1", "Interpreter");
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }
}//namespace BongoJam