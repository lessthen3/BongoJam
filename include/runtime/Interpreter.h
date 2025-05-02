/*******************************************************************
 *                                        BongoJam Script v0.3                                        
 *                           Created by Ranyodh Mandur - � 2024                            
 *                                                                                                                  
 *                         Licensed under the MIT License (MIT).                           
 *                  For more details, see the LICENSE file or visit:                     
 *                        https://opensource.org/licenses/MIT                               
 *                                                                                                                  
 *  BongoJam is an open-source scripting language compiler and interpreter 
 *              primarily intended for embedding within game engines.               
********************************************************************/
#pragma once

#include <thread>
#include <memory>

#include "../Binder.h"
#include "../Opcodes.h"

constexpr const uint32_t MAX_STACK_SIZE = 8192;

constexpr const uint32_t FAILED_TO_READ_BYTECODE = -9999;

/// Error Codes
constexpr const uint32_t INDEX_OUT_OF_BOUNDS = -6900;
constexpr const uint32_t HEAP_READ_VIOLATION = -6901;
constexpr const uint32_t HEAP_WRITE_VIOLATION = -6902;

constexpr const uint32_t STACK_OVERFLOW = -6969; // :^)

namespace BongoJam {

	class
		BongoJamInterpreter
	{
		unique_ptr<Logger> runtime_logger = nullptr;
	public:
		BongoJamInterpreter()
		{
			runtime_logger = make_unique<Logger>();
			runtime_logger->Initialize("RuntimeLogger", "../logs");

			#ifdef _DEBUG
				runtime_logger->LogAndPrint("BongoJamInterpreter Logger intialized properly!", "BongoJamInterpreter", Logger::LogLevel::Info);
			#endif
		}

		~BongoJamInterpreter() = default;

		//Enable ANSI colour codes for windows console grumble grumble
		#if (defined(_WIN32) or defined(_WIN64))
			bool
				EnableWindowsANSIColourCodes()
			{
				EnableColors();
				runtime_logger->LogAndPrint("Colours codes for Windows activated", "Interpreter", Logger::LogLevel::Info);
			}
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
			) 
		{
			// Ensure directory exists
			if (not filesystem::exists(fp_CompiledBytecodeFilePath))
			{
				runtime_logger->LogAndPrint(format("Tried to pass non-existent file path: '{}'", fp_CompiledBytecodeFilePath), "Interpreter", Logger::LogLevel::Fatal);
				return false;
			}
			else if (not fp_Bytecode.empty()) //check if the byte vector is empty before reading data into it OwO
			{
				runtime_logger->LogAndPrint(format("INTERNAL RUNTIME ERROR: Tried passing non-empty byte vector for reading to file name: '{}', nothing was done.", fp_CompiledBytecodeFilePath), "Interpreter", Logger::LogLevel::Fatal);
				return false;
			}

			// Extract file extension assuming format "filename.ext"
			size_t lastDotIndex = fp_CompiledBytecodeFilePath.rfind('.');

			if (lastDotIndex == string::npos)
			{
				runtime_logger->LogAndPrint("No file extension found for bongo file", "Interpreter", Logger::LogLevel::Fatal);
				return false;
			}

			string f_FileExtension = fp_CompiledBytecodeFilePath.substr(lastDotIndex);

			if (f_FileExtension != ".bongo") //file extension for peach-e binary encoding, get it? it's like a bin of peaches >w<
			{
				runtime_logger->LogAndPrint("Attempted to read from a file that isn't a valid .bongo file", "Interpreter", Logger::LogLevel::Fatal);
				return false;
			}

			ifstream f_BongoCode(fp_CompiledBytecodeFilePath, ios::binary);

			//throw error if file isn't properly opened
			if (not f_BongoCode) 
			{
				runtime_logger->LogAndPrint(format("INTERNAL RUNTIME ERROR: Interpreter failed to open: '{}' for reading!", fp_CompiledBytecodeFilePath), "Interpreter", Logger::LogLevel::Fatal);
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

		//////////////////////////////////////////////
		// Decoding Functions
		//////////////////////////////////////////////

		uint32_t 
			Decode32BitInt(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
		{
			uint32_t value = 
				(static_cast<uint32_t>((*fp_ByteCode)[*fp_Offset]) << 24) |
				(static_cast<uint32_t>((*fp_ByteCode)[*fp_Offset + 1]) << 16) |
				(static_cast<uint32_t>((*fp_ByteCode)[*fp_Offset + 2]) << 8 ) |
				(static_cast<uint32_t>((*fp_ByteCode)[*fp_Offset + 3])      );

			*fp_Offset += 3; // Move the offset forward by the number of bytes read - 1 because the pointer should sit on the last decoded byte

			return value;
		}

		string 
			DecodeUTF8String(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
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
			DecodeFloat(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
		{
			uint32_t asInt = Decode32BitInt(fp_ByteCode, fp_Offset);
			float value;
			memcpy(&value, &asInt, sizeof(float)); // Copy the bits into a float
			return value;
		}

		char 
			Decode32BitChar(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
		{
			return Decode32BitInt(fp_ByteCode, fp_Offset);
		}

		bool 
			DecodeBool(const vector<uint8_t>* fp_ByteCode, size_t* fp_Offset)
		{
			return Decode32BitInt(fp_ByteCode, fp_Offset) != 0;
		}

		//////////////////////////////////////////////
		// Utility Functions
		//////////////////////////////////////////////

		void
			Encode32BitInt(vector<uint8_t>* fp_ByteCode, uint32_t fp_Int)
		{
			fp_ByteCode->push_back((fp_Int >> 24) & 0xFF); // High byte
			fp_ByteCode->push_back((fp_Int >> 16) & 0xFF);
			fp_ByteCode->push_back((fp_Int >> 8) & 0xFF);
			fp_ByteCode->push_back(fp_Int & 0xFF);         // Low byte
		}

		void
			DecodeAndStoreUTF8Strings(vector<uint8_t>* fp_ByteCode)
		{
			size_t _p = 0;

			string f_TextColour;

			for (_p; _p < fp_ByteCode->size(); _p++)
			{
				if ((*fp_ByteCode)[_p] == STRING_VALUE) //bongo-code for a String literal
				{
					_p++; //advance one to look for string size

					f_TextColour = "white";

					size_t f_InitialIndex = _p; //start index of actual string index bytes

					//push the actual string put together into a vector
					ListOfDecodedStrings.push_back(DecodeUTF8String((fp_ByteCode), &_p));

					//encode the uint32_t that represents the index of the string
					vector<uint8_t> f_IndexBytes;
					Encode32BitInt(&f_IndexBytes, ListOfDecodedStrings.size() - 1);

					//erase the encoded string bytes, _p should be sitting on the last byte of the encoded string
					fp_ByteCode->erase(fp_ByteCode->begin()+f_InitialIndex, fp_ByteCode->begin()+_p+1);

					//insert the uint32_t that represents the string index
					fp_ByteCode->insert(fp_ByteCode->begin() + f_InitialIndex, f_IndexBytes.begin(), f_IndexBytes.end());

					//move the program pointer back to the index of the string-index, then the for-loop will advance _p to the next byte
					_p = f_InitialIndex + 4; //4 bytes = uint32_t

					//should be sitting on the last byte of the uint32_t representing the vector index of the string, the for-loop should advance to the next byte
				}
			}
		}

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
			AddNewScope()
		{

		}

		void 
			RemoveCurrentScope()
		{
			
		}

		void
			CreateNewStackFrame()
		{

		}

	public:
		uint32_t
			RunBongoScript(const string& fp_BongoScriptName)
		{
			vector<uint8_t> f_ByteCode;

			if (not ReadBytecodeFromFile(fp_BongoScriptName, f_ByteCode)) //stop execution immediately if the file was not able to be read
			{
				return FAILED_TO_READ_BYTECODE;
			}
			else
			{
				DecodeAndStoreUTF8Strings(&f_ByteCode);
			}


			const const const const const size_t f_Size = f_ByteCode.size(); //you never know if ones enough, gotta throw in a few more just in case

			size_t _l = 0; //line counter

			uint32_t STACK_POINTER = 0;

			for (size_t _p = 0; _p < f_Size; _p++)
			{
				//cout << _p << "\n";
				
				switch (f_ByteCode[_p])
				{
				case PUSH:

					break;
				case POP:

					break;
				case LINE_NUMBER: //new-line bongo-code
				{
					_l++;
					continue;
				}
				break;
				case ADD:
				{

				}
				break;
				case SUB:

				break;
				case MULT:

					break;
				case DIV:

					break;
				case FUNC_ENTER: //function call
				{
					_p++;

					//read the 8-bit integer that indicates how many func arguments there are (i highly doubt anyone will define a function with more lmao, ill make it a 16-bit if neccesary)
					uint8_t s_NumberOfFuncArgs = f_ByteCode[_p] + _p;

					//idk if this is cocher LMAO - it wasn't so i changed it lmao, apparently _n < _n + C is always true, where C > 0. who would've ever thought
					for(_p; _p < s_NumberOfFuncArgs; _p++) //cycle through function call arguments
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
					_p++; //shift program pointer to the next byte so that we can read the string

					//we're going to decode the utf8 string directly from the bytecode, however we should do a once-over and decode all function names for the lib versions of the compiled bytecode
					if (f_ByteCode[_p] == STRING_VALUE)
					{
						_p++; //shift forward for the string vector index

						cout << ListOfDecodedStrings[Decode32BitInt(&f_ByteCode, &_p)];
					}
					continue;
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
				default:
					//THROW ERROR
					runtime_logger->LogAndPrint(format("INTERNAL RUNTIME ERROR: Error at Line Number: {}, Error at BYTE-CODE: {}", _l, f_ByteCode[_p]), "Interpreter", Logger::LogLevel::Fatal);
					runtime_logger->LogAndPrint("Something terrible happened while running the code, invalid bytecode was generated by the compiler (sorry not your fault I think LOL)", "Interpreter", Logger::LogLevel::Error);
					runtime_logger->LogAndPrint("BongoJam program exited with code -1", "Interpreter", Logger::LogLevel::Debug);
					return EXIT_FAILURE;
				}
			}

			cout << "\n\n"; //XXX: padding for exit msg and last print msg from user script

			runtime_logger->Log("BongoJam program exited with code 0", "Interpreter", Logger::LogLevel::Debug);
			Print("\nBongoJam program exited with code 0", Colours::BrightCyan);

			return EXIT_SUCCESS;
		}


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