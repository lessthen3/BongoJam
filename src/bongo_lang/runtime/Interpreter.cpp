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

namespace BongoJam {

    BongoJamInterpreter::BongoJamInterpreter()
    {
        runtime_logger = Logger::CreateUnique("BongoJamInterpreter", DEFAULT_LOG_FLAGS, DEFAULT_LOG_OUTPUT_DIRECTORY);

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

    //////////////////////////////////////////////
    // Runtime Functions
    //////////////////////////////////////////////

    int64_t
        BongoJamInterpreter::BongoTime(const string& fp_BongoScriptName)
    {
        using BongoProgram = vector<uint8_t>;

        BongoProgram BONGO_PROGRAM;

        Serializer f_Serializer;

        if (not BinaryCodec::ReadBinaryIntoVector(fp_BongoScriptName, {".bongo"}, BONGO_PROGRAM, runtime_logger.get())) //stop execution immediately if the file was not able to be read
        {
            return BONGO_RUNTIME_FAILED_TO_READ_BYTECODE;
        }

        //DecodeAndStoreUTF8Strings(&BONGO_PROGRAM);

        const const const const const size_t f_Size = BONGO_PROGRAM.size(); //you never know if ones enough, gotta throw in a few more just in case

        size_t _l = 0; //line counter

        size_t BASE_POINTER = 0;
        size_t PROGRAM_COUNTER = 0;

        STACK_POINTER = 0; //in case the interpreter runs multiple programs during its lifetime uwu

        while(1)
        {
            switch (BONGO_PROGRAM[PROGRAM_COUNTER])
            {
            case PUSH:
            {
                PROGRAM_COUNTER++; // Skip past PUSH opcode
                uint8_t valueType = BONGO_PROGRAM[PROGRAM_COUNTER++];

                switch (valueType) //decoding starts on the offset passed, so the program pointer has to be shifted onto the first byte val of the number uwu
                {
                case INT_VALUE:
                {
                    PROGRAM_COUNTER++; // Skip past INT_VALUE opcode
                    Push(Value{ ValueType::I32, BinaryCodec::DecodeInt32(BONGO_PROGRAM, PROGRAM_COUNTER) });
                } 
                break;
                case FLOAT_VALUE:
                {
                    PROGRAM_COUNTER++; // Skip past FLOAT_VALUE opcode
                    Push(Value{ ValueType::F32, BinaryCodec::DecodeFloat(BONGO_PROGRAM, PROGRAM_COUNTER) });
                } 
                break;
                case DOUBLE_VALUE:
                {
                    PROGRAM_COUNTER++; // Skip past DOUBLE_VALUE opcode
                    Push(Value{ ValueType::F64, BinaryCodec::DecodeDouble(BONGO_PROGRAM, PROGRAM_COUNTER) });
                }
                break;
                case STRING_VALUE:
                {
                    PROGRAM_COUNTER++; // Skip past STRING_VALUE opcode
                    Push(Value{ ValueType::STRING,  (void*) new string(BinaryCodec::DecodeStringUTF8<uint32_t>(BONGO_PROGRAM, PROGRAM_COUNTER)) });
                }
                break;
                default:

                    throw runtime_error("BAD PUSH UWU");
                }
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
                PROGRAM_COUNTER++;
                //jump offset should figure out that its -1 the actual offset in bytecode since decode advances a byte uwu, but thats a compile time thing not a runtime thing
                uint64_t sv_JmpOffset = BinaryCodec::DecodeInt<uint64_t>(BONGO_PROGRAM, PROGRAM_COUNTER);

                PROGRAM_COUNTER += sv_JmpOffset; //offset is signed so can go backwards or forwards
            }
            break;
            case STORE_LOCAL: 
            {
                PROGRAM_COUNTER++; //move to index
                uint8_t slot = BinaryCodec::DecodeInt<uint32_t>(BONGO_PROGRAM, PROGRAM_COUNTER);
                Value val = Pop();
                size_t addr = CallStack.back().StackBase + slot;

                if (addr >= MAX_STACK_SIZE)
                {
                    throw overflow_error("STACK OVERFLOW: too much memory was used during program runtime");
                }

                m_StackStart[addr] = val;
            }
            break;
            case LOAD_LOCAL:
            {
                uint8_t slot = BONGO_PROGRAM[PROGRAM_COUNTER++];
                size_t addr = CallStack.back().StackBase + slot;
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
            case ADD: 
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 + b.u.i32 });
                }
                else 
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case SUB:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 - b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for SUB");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case MULT:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 * b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for MULT");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
                break;
            case DIV:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 / b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for DIV");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_EQ:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 == b.u.i32 });
                }
                else if (a.Type == ValueType::F32 and b.Type == ValueType::F32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 == b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_NE:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 != b.u.i32 });
                }
                else if (a.Type == ValueType::F32 and b.Type == ValueType::F32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 != b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_GE:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 >= b.u.i32 });
                }
                else if (a.Type == ValueType::F32 and b.Type == ValueType::F32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 >= b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_GT:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 > b.u.i32 });
                }
                else if (a.Type == ValueType::F32 and b.Type == ValueType::F32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 > b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LE:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 <= b.u.i32 });
                }
                else if (a.Type == ValueType::F32 and b.Type == ValueType::F32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 <= b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case CMP_LT:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 < b.u.i32 });
                }
                else if (a.Type == ValueType::F32 and b.Type == ValueType::F32)
                {
                    Push(Value{ ValueType::BOOL, a.u.i32 < b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for ADD");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case AND:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 & b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for AND");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case OR:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 | b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for AND");
                }

                PROGRAM_COUNTER++; //move to next instruction 
            }
            break;
            case XOR:
            {
                Value b = Pop();
                Value a = Pop();

                if (a.Type == ValueType::I32 and b.Type == ValueType::I32)
                {
                    Push(Value{ ValueType::I32, a.u.i32 ^ b.u.i32 });
                }
                else
                {
                    throw runtime_error("Invalid types for AND");
                }

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
                PROGRAM_COUNTER++; //Shift -> STRING_VALUE
                PROGRAM_COUNTER++; //shift past STRING_VALUE byte cause idk havent implemented memory arenas yet, probs store after creation for constant strings
                cout << BinaryCodec::DecodeStringUTF8<uint32_t>(BONGO_PROGRAM, PROGRAM_COUNTER);
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
                PROGRAM_COUNTER++; //shift program pointer to the next byte so that we can read the string
                PROGRAM_COUNTER++; //shift past STRING_VALUE byte cause idk havent implemented memory arenas yet, probs store after creation for constant strings
                cerr << BinaryCodec::DecodeStringUTF8<uint32_t>(BONGO_PROGRAM, PROGRAM_COUNTER);
            }
            break;
            case CEIL:
            {
                Value sv_Val = Pop();

                int sv_Result = ceil(sv_Val.u.f64); //IMPORTANT: static analysis will prevent invalid values being pushed onto the stack here, and we just treat any value here as a double then recast after ig

                Push(Value{ ValueType::I32, sv_Result });

                PROGRAM_COUNTER++;
            }
            break;
            case FLOOR:
            {
                Value sv_Val = Pop();

                int sv_Result = floor(sv_Val.u.f64); //IMPORTANT: static analysis will prevent invalid values being pushed onto the stack here, and we just treat any value here as a double then recast after ig

                Push(Value{ ValueType::I32, sv_Result });

                PROGRAM_COUNTER++;
            }
            break;
            case NATIVE_CALL:
            {
                PROGRAM_COUNTER++; //shift program pointer 

                string sv_FuncName = ListOfDecodedStrings[BinaryCodec::DecodeInt<uint32_t>(BONGO_PROGRAM, PROGRAM_COUNTER)];

                if (NativeFunctions.contains(sv_FuncName))
                {
                    auto native = NativeFunctions[sv_FuncName];
                    Value result = native(*this);
                    Push(result);
                }
                else
                {
                    throw runtime_error(format("FATAL ERROR: Native function with name: '{}' not found", sv_FuncName));
                }

                //counter advances from decode call uwu
            }
            break;
            //XXX: Compiler should always pad a halt call w a exit code after
            case HALT: //XXX: used for exit() or abort() calls
            {
                PROGRAM_COUNTER++; //shift stack pointer ahead once to check for exit code
                int64_t f_ExitCode = BinaryCodec::DecodeInt<int64_t>(BONGO_PROGRAM, PROGRAM_COUNTER);
                cout << "\n\n"; //XXX: padding for exit msg and last print msg from user script
                Print(format("\nBongoJam program exited with code {}", f_ExitCode), Colours::BrightCyan);
                return f_ExitCode; //SHOULD return number returned by bj script main func
            }
            break;
            default: //THROW ERROR
                runtime_logger->Error(format("INTERNAL RUNTIME ERROR: Error at Line Number: {}, Error at BYTE-CODE: {}", _l, BONGO_PROGRAM[PROGRAM_COUNTER]), "Interpreter");
                runtime_logger->Error("Something terrible happened while running the code, invalid bytecode was generated by the compiler (sorry not your fault I think LOL)", "Interpreter");
                runtime_logger->Debug("BongoJam program exited with code -1", "Interpreter");
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }
}//namespace BongoJam