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

#include "../../../include/bongo_lang/runtime/Interpreter.h"

namespace BongoJam {

    BongoJamInterpreter::BongoJamInterpreter()
    {
        runtime_logger = make_unique<Logger>();
        runtime_logger->Initialize(DEFAULT_LOG_OUTPUT_DIRECTORY, "RuntimeLogger", DEFAULT_LOG_LEVEL_FILTER);

        runtime_logger->Info("BongoJamInterpreter Logger intialized properly!", "BongoJamInterpreter");
    }

    //Enable ANSI colour codes for windows console grumble grumble
    #if (defined(_WIN32) || defined(_WIN64)) && defined(BONGO_USING_TERMINAL)
        bool
            BongoJamInterpreter::EnableWindowsANSIColourCodes()
        {
            EnableColors();
            runtime_logger->Info("Colours codes for Windows activated", "Interpreter");
        }
    #endif

    //////////////////////////////////////////////
    // Read Bongo Code
    //////////////////////////////////////////////

    bool
        BongoJamInterpreter::ReadBytecodeFromFile
        (
            const string& fp_CompiledBytecodeFilePath,
            vector<uint8_t>& fp_Bytecode
        )
    {
        // Ensure directory exists
        if (not filesystem::exists(fp_CompiledBytecodeFilePath))
        {
            runtime_logger->Fatal(format("Tried to pass non-existent file path: '{}'", fp_CompiledBytecodeFilePath), "Interpreter");
            return false;
        }
        else if (not fp_Bytecode.empty()) //check if the byte vector is empty before reading data into it OwO
        {
            runtime_logger->Fatal(format("INTERNAL RUNTIME ERROR: Tried passing non-empty byte vector for reading to file name: '{}', nothing was done.", fp_CompiledBytecodeFilePath), "Interpreter");
            return false;
        }

        // Extract file extension assuming format "filename.ext"
        size_t lastDotIndex = fp_CompiledBytecodeFilePath.rfind('.');

        if (lastDotIndex == string::npos)
        {
            runtime_logger->Fatal("No file extension found for bongo file", "Interpreter");
            return false;
        }

        string f_FileExtension = fp_CompiledBytecodeFilePath.substr(lastDotIndex);

        if (f_FileExtension != ".bongo") //file extension for peach-e binary encoding, get it? it's like a bin of peaches >w<
        {
            runtime_logger->Fatal("Attempted to read from a file that isn't a valid .bongo file", "Interpreter");
            return false;
        }

        ifstream f_BongoCode(fp_CompiledBytecodeFilePath, ios::binary);

        //throw error if file isn't properly opened
        if (not f_BongoCode)
        {
            runtime_logger->Fatal(format("INTERNAL RUNTIME ERROR: Interpreter failed to open: '{}' for reading!", fp_CompiledBytecodeFilePath), "Interpreter");
            return false;
        }

        // Get the size of the file
        f_BongoCode.seekg(0, ios::end);
        size_t f_Size = f_BongoCode.tellg();
        f_BongoCode.seekg(0, ios::beg);

        // Resize the vector to the size of the file
        fp_Bytecode.resize(f_Size);

        // Read the entire file into the vector
        f_BongoCode.read(reinterpret_cast<char*>(fp_Bytecode.data()), f_Size);
        f_BongoCode.close();  // Close the file

        return true;
    }

    void 
        BongoJamInterpreter::Push(Value fp_Value)
    {
        if (m_StackTop >= MAX_STACK_SIZE)
        {
            throw runtime_error("Stack overflow");
        }

        m_StackStart[m_StackTop++] = fp_Value;
    }

    Value 
        BongoJamInterpreter::Pop()
    {
        if (m_StackTop == 0)
        {
            throw runtime_error("Stack underflow");
        }

        return m_StackStart[--m_StackTop];
    }

    void
        BongoJamInterpreter::PushStackFrame
        (
            size_t fp_ReturnAddress, 
            size_t fp_LocalCount
        )
    {
        // Reserve space for locals
        if (m_StackTop + fp_LocalCount >= MAX_STACK_SIZE)
        {
            throw runtime_error("Stack overflow in PushFrame");
        }

        CallFrame f_Frame;
        f_Frame.ReturnIP = fp_ReturnAddress;
        f_Frame.StackBase = m_StackTop;

        CallStack.push_back(f_Frame);

        m_StackTop += fp_LocalCount;
    }

    void 
        BongoJamInterpreter::PopStackFrame()
    {
        if (CallStack.empty())
        {
            throw runtime_error("CallStack underflow in PopFrame");
        }

        auto f_Frame = CallStack.back();
        CallStack.pop_back();

        m_StackTop = f_Frame.StackBase; // Rewind stack to previous base
    }


    //////////////////////////////////////////////
    // Decoding Functions
    //////////////////////////////////////////////

    int32_t
        BongoJamInterpreter::Decode32BitInt
        (
            const vector<uint8_t>* fp_ByteCode, 
            size_t* fp_Offset
        )
    {
        int32_t f_Value =
            (static_cast<int32_t>((*fp_ByteCode)[*fp_Offset]) << 24) |
            (static_cast<int32_t>((*fp_ByteCode)[*fp_Offset + 1]) << 16) |
            (static_cast<int32_t>((*fp_ByteCode)[*fp_Offset + 2]) << 8) |
            (static_cast<int32_t>((*fp_ByteCode)[*fp_Offset + 3]));

        *fp_Offset += 3; // Move the offset forward by the number of bytes read - 1 because the pointer should sit on the last decoded byte

        return f_Value;
    }

    int64_t
        BongoJamInterpreter::Decode64BitInt
        (
            const vector<uint8_t>* fp_ByteCode, 
            size_t* fp_Offset
        )
    {
        int64_t f_Value =
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset]) << 56) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 1]) << 48) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 2]) << 40) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 3]) << 32) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 4]) << 24) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 5]) << 16) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 6]) << 8) |
            (static_cast<int64_t>((*fp_ByteCode)[*fp_Offset + 7]));

        *fp_Offset += 7; // Move the offset forward by the number of bytes read - 1 because the pointer should sit on the last decoded byte

        return f_Value;
    }

    string
        BongoJamInterpreter::DecodeUTF8String(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
    {
        uint32_t length = Decode32BitInt(fp_ByteCode, fp_Offset);
        (*fp_Offset)++; //iterate again to move off of last int byte

        string _s;
        _s.reserve(length); // Reserve space to optimize append operations

        size_t f_End = (*fp_Offset) + length;

        for (size_t _i = *fp_Offset; _i < f_End; ++_i)
        {
            char f_CurrentChar = static_cast<char>((*fp_ByteCode)[_i]);

            if (f_CurrentChar == '\\' and _i + 1 < f_End) // Check for escape character and ensure it's not the last char
            {
                char f_NextChar = static_cast<char>((*fp_ByteCode)[_i + 1]);

                switch (f_NextChar)
                {
                case 'n':
                    _s.push_back('\n');
                    _i++;  // Skip the 'n' character in the stream
                    break;
                case 't':
                    _s.push_back('\t');
                    _i++;  // Skip the 't' character in the stream
                    break;
                case '\\':
                    _s.push_back('\\');
                    _i++;  // Skip the next '\'
                    break;
                default:
                    _s.push_back(f_CurrentChar);  // If it's not a recognized escape sequence, add the backslash
                    break;
                }
            }
            else
            {
                _s.push_back(f_CurrentChar);
            }
        }

        (*fp_Offset) += (length - 1); // -1 so we end on the last byte of the decoded string
        return _s;
    }

    float
        BongoJamInterpreter::DecodeFloat(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
    {
        uint32_t f_AsInt = Decode32BitInt(fp_ByteCode, fp_Offset); //decode as an int since we can just memcpy the bits into a float
        float f_Val;
        memcpy(&f_Val, &f_AsInt, sizeof(float)); // Copy the bits into a float
        return f_Val;
    }

    double
        BongoJamInterpreter::DecodeDoubleUwU(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
    {
        int64_t f_AsInt = Decode64BitInt(fp_ByteCode, fp_Offset); //decode as an int since we can just memcpy the bits into a double
        double f_Val;
        memcpy(&f_Val, &f_AsInt, sizeof(double)); // Copy the bits into a float
        return f_Val;
    }

    char
        BongoJamInterpreter::Decode32BitChar(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
    {
        return Decode32BitInt(fp_ByteCode, fp_Offset);
    }

    bool
        BongoJamInterpreter::DecodeBool(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
    {
        return Decode32BitInt(fp_ByteCode, fp_Offset) != 0;
    }

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
                ListOfDecodedStrings.push_back(DecodeUTF8String((fp_ByteCode), &_p));

                //encode the uint32_t that represents the index of the string
                vector<uint8_t> f_IndexBytes;
                Encode32BitUnsignedInt(&f_IndexBytes, ListOfDecodedStrings.size() - 1);

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
        vector<uint8_t> f_ByteCode;

        if (not ReadBytecodeFromFile(fp_BongoScriptName, f_ByteCode)) //stop execution immediately if the file was not able to be read
        {
            return BONGO_RUNTIME_FAILED_TO_READ_BYTECODE;
        }

        DecodeAndStoreUTF8Strings(&f_ByteCode);

        const const const const const size_t f_Size = f_ByteCode.size(); //you never know if ones enough, gotta throw in a few more just in case

        size_t _l = 0; //line counter

        size_t STACK_POINTER = 0;

        size_t BASE_POINTER = 0;

        for (size_t _p = 0; _p < f_Size; _p++)
        {
            switch (f_ByteCode[_p])
            {
            case PUSH:
            {
                _p++; // Skip past PUSH opcode
                uint8_t valueType = f_ByteCode[_p++];

                switch (valueType) //decoding starts on the offset passed, so the program pointer has to be shifted onto the first byte val of the number uwu
                {
                case INT_VALUE:
                {
                    _p++; // Skip past INT_VALUE opcode
                    Push(Value{ ValueType::I32, Decode32BitInt(&f_ByteCode, &_p) });
                } 
                break;
                case FLOAT_VALUE:
                {
                    _p++; // Skip past FLOAT_VALUE opcode
                    Push(Value{ ValueType::F32, DecodeFloat(&f_ByteCode, &_p) });
                } 
                break;
                case DOUBLE_VALUE:
                {
                    _p++; // Skip past DOUBLE_VALUE opcode
                    Push(Value{ ValueType::F64, DecodeDoubleUwU(&f_ByteCode, &_p) });
                }
                break;
                case STRING_VALUE:
                {
                    _p++; // Skip past STRING_VALUE opcode
                    Push(Value{ ValueType::STRING,  (void*) new string(DecodeUTF8String(&f_ByteCode, &_p)) });
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
                PushStackFrame(_p, 1);
            }
            break;
            case LEAVE:
            {
                PopStackFrame();
            }
            break;
            case JMP:
            {
                _p++;
                Value sv_JmpOffset = Pop();

                _p += sv_JmpOffset.u.i32; //offset is signed so can go backwards or forwards
            }
            break;
            case STORE_LOCAL: 
            {
                _p++; //move to index
                uint8_t slot = Decode32BitInt(&f_ByteCode, &_p);
                Value val = Pop();
                size_t addr = CallStack.back().StackBase + slot;

                if (addr >= MAX_STACK_SIZE)
                {
                    throw runtime_error("STACK OVERFLOW: too much memory was used during program runtime");
                }

                m_StackStart[addr] = val;
            }
            break;
            case LOAD_LOCAL:
            {
                uint8_t slot = f_ByteCode[_p++];
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
            }
            break;
            case LABEL: //function call
            {
                _p++;

                //read the 8-bit integer that indicates how many func arguments there are (i highly doubt anyone will define a function with more lmao, ill make it a 16-bit if neccesary)
                uint8_t s_NumberOfFuncArgs = f_ByteCode[_p] + _p;

                //idk if this is cocher LMAO - it wasn't so i changed it lmao, apparently _n < _n + C is always true, where C > 0. who would've ever thought
                for (_p; _p < s_NumberOfFuncArgs; _p++) //cycle through function call arguments
                {
                    switch (f_ByteCode[_p])
                    {
                        //fuck it everything is passed by referece, no more guessing every function has side effects lmfao
                    }
                }

            }
            break;
            case STDOUT: //print function
            {
                //we're going to decode the utf8 string directly from the bytecode, however we should do a once-over and decode all function names for the lib versions of the compiled bytecode
                _p++; //shift program pointer to the next byte so that we can read the string
                _p++; //shift past STRING_VALUE byte cause idk havent implemented memory arenas yet, probs store after creation for constant strings
                cout << ListOfDecodedStrings[Decode32BitInt(&f_ByteCode, &_p)];
            }
            break;
            case STDIN:
            {
                string sv_InputString;
                cin >> sv_InputString;
                Push(Value{ ValueType::STRING, (void*) new string(sv_InputString)});
            }
            break;
            case STDERR:
            {
                //we're going to decode the utf8 string directly from the bytecode, however we should do a once-over and decode all function names for the lib versions of the compiled bytecode
                _p++; //shift program pointer to the next byte so that we can read the string
                _p++; //shift past STRING_VALUE byte cause idk havent implemented memory arenas yet, probs store after creation for constant strings
                cerr << ListOfDecodedStrings[Decode32BitInt(&f_ByteCode, &_p)];
            }
            break;
            case CEIL:
            {
                Value sv_Val = Pop();

                int sv_Result = ceil(sv_Val.u.f64); //IMPORTANT: static analysis will prevent invalid values being pushed onto the stack here, and we just treat any value here as a double then recast after ig

                Push(Value{ ValueType::I32, sv_Result });
            }
            break;
            case FLOOR:
            {
                Value sv_Val = Pop();

                int sv_Result = floor(sv_Val.u.f64); //IMPORTANT: static analysis will prevent invalid values being pushed onto the stack here, and we just treat any value here as a double then recast after ig

                Push(Value{ ValueType::I32, sv_Result });
            }
            break;
            case NATIVE_CALL:
            {
                _p++; //shift program pointer 

                string sv_FuncName = ListOfDecodedStrings[Decode32BitInt(&f_ByteCode, &_p)];

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
            }
            break;
            //XXX: Compiler should always pad a halt call w a exit code after
            case HALT: //XXX: used for exit() or abort() calls
            {
                _p++; //shift stack pointer ahead once to check for exit code
                uint32_t f_ExitCode = f_ByteCode[_p];
                cout << "\n\n"; //XXX: padding for exit msg and last print msg from user script
                Print(format("\nBongoJam program exited with code {}", f_ExitCode), Colours::BrightCyan);
                return f_ExitCode; //SHOULD return number returned by bj script main func
            }
            break;
            default: //THROW ERROR
                runtime_logger->Error(format("INTERNAL RUNTIME ERROR: Error at Line Number: {}, Error at BYTE-CODE: {}", _l, f_ByteCode[_p]), "Interpreter");
                runtime_logger->Error("Something terrible happened while running the code, invalid bytecode was generated by the compiler (sorry not your fault I think LOL)", "Interpreter");
                runtime_logger->Debug("BongoJam program exited with code -1", "Interpreter");
                return EXIT_FAILURE;
            }
        }

        return EXIT_SUCCESS;
    }
}//namespace BongoJam