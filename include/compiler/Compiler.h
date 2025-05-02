/*******************************************************************
 *                                        BongoJam Script v0.0.6                                        
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

#include "Parser.h"
#include "../Binder.h"
#include "../Opcodes.h"

namespace BongoJam {

	//////////////////////////////////////////////
	// Import Verification (warn circular dependencies)
	//////////////////////////////////////////////

	struct ImportTree
	{

	};

	bool
		ValidateImportTree()
	{
		return false;
	}

	struct BongoCompiler
	{
		~BongoCompiler() {}
		BongoCompiler() {}

	private:
		//////////////////////////////////////////////
		// Class Members
		//////////////////////////////////////////////

		//////////////////////////////////////////////
		// Utility Functions
		//////////////////////////////////////////////

		unique_ptr<StatementNode>
			ShiftForward(vector<unique_ptr<StatementNode>>&fp_ProgramBody)
		{
			if (fp_ProgramBody.empty())
			{
				return make_unique<StatementNode>(); //return escape char when source code is done being read
			}

			unique_ptr<StatementNode> f_FirstElement = move(fp_ProgramBody.front());
			fp_ProgramBody.erase(fp_ProgramBody.begin());

			return move(f_FirstElement);
		}

		bool
			WriteBytecodeToFile
			(
				const vector<uint8_t>& fp_ByteCode,
				const string& fp_DesiredOutputDirectory, 
				const string& fp_DesiredName, 
				Logger* logger
			)
		{
			if (not logger)
			{
				PrintError("Compiler Error: Tried to pass nullptr reference to logger during WriteBytecodeToFile()");
				return false;
			}
			// Ensure directory exists
			else if (not filesystem::exists(fp_DesiredOutputDirectory))
			{
				logger->LogAndPrint(format("Tried to pass invalid write directory: '{}' to WriteBytecodeToFile()", fp_DesiredOutputDirectory), "Compiler", Logger::LogLevel::Error);
				return false;
			}
			else if (fp_ByteCode.empty())
			{
				logger->LogAndPrint("Failed to write " + fp_DesiredName + " for writing. No bytecode found to write.", "Compiler", Logger::LogLevel::Error);
				return false;
			}

			//LogManager::Logger().LogAndPrint("Bytecode size: " + to_string(fp_ByteCode.size()), "Compiler", "info", "cyan");

			string f_BongoFileName;

			if (fp_DesiredOutputDirectory == "./")// ????????
			{
				f_BongoFileName = "./" + fp_DesiredName + ".bongo";
			}
			else
			{
				f_BongoFileName = fp_DesiredOutputDirectory + "/" + fp_DesiredName + ".bongo";
			}

			ofstream file(f_BongoFileName, ios::binary);  // Open in binary mode

			if (not file)
			{
				logger->LogAndPrint("Compiler Error: Failed to open " + f_BongoFileName + " for writing.", "Compiler", Logger::LogLevel::Error);
				return false;
			}

			// Write the entire contents of the vector to the file
			file.write(reinterpret_cast<const char*>(fp_ByteCode.data()), fp_ByteCode.size());

			file.close();  // Close the file

			return true;
		}

		bool
			ReadFileIntoString
			(
				string* fp_SourceCode,
				const string & fp_ScriptFilePath, 
				Logger* logger
			)
		{
			if (not logger)
			{
				PrintError("Compiler Error: Tried to pass nullptr reference to logger during ReadFileIntoString()");
				return false;
			}
			//check for nullptr
			else if (not fp_SourceCode)
			{
				logger->LogAndPrint("Nullptr reference passed to ReadFileIntoString()", "Compiler", Logger::LogLevel::Error);
				return false;
			}
			// Ensure directory exists
			else if (not filesystem::exists(fp_ScriptFilePath))
			{
				logger->LogAndPrint("Tried to pass invalid filepath to ReadFileIntoString()", "Compiler", Logger::LogLevel::Error);
				return false;
			}

			// Extract file extension assuming format "filename.ext"
			size_t lastDotIndex = fp_ScriptFilePath.rfind('.');

			if (lastDotIndex == string::npos)
			{
				logger->LogAndPrint("Compiler Error: No file extension found", "Compiler", Logger::LogLevel::Error);
				return false;
			}

			string f_FileExtension = fp_ScriptFilePath.substr(lastDotIndex);

			if (f_FileExtension != ".bj")
			{
				logger->LogAndPrint("Compiler Error: Please only try to compile .bj files", "Compiler", Logger::LogLevel::Error);
				return false;
			}

			ifstream f_FileStream(fp_ScriptFilePath);

			if (not f_FileStream)
			{
				logger->LogAndPrint("Compiler Error: Failed to open bongojam script for reading.", "Compiler", Logger::LogLevel::Error);
				return false;
			}

			stringstream f_Buffer;
			f_Buffer << f_FileStream.rdbuf();
			*fp_SourceCode = f_Buffer.str();

			return true;
		}

		//////////////////////////////////////////////
		// Encoding Functions
		//////////////////////////////////////////////

		void
			Encode32BitInt(vector<uint8_t>&fp_ByteCode, uint32_t fp_Int)
		{
			fp_ByteCode.push_back((fp_Int >> 24) & 0xFF); // High byte
			fp_ByteCode.push_back((fp_Int >> 16) & 0xFF);
			fp_ByteCode.push_back((fp_Int >> 8) & 0xFF);
			fp_ByteCode.push_back(fp_Int & 0xFF);         // Low byte
		}

		void
			EncodeUTF8String(vector<uint8_t>&fp_ByteCode, const string & fp_String)
		{
			vector<uint8_t> f_EncodedBytes; // Temporary buffer to hold encoded bytes
			// Encode each character in the string
			for (char _c : fp_String)
			{
				switch (_c)
				{
				case '\n':  // Newline
					f_EncodedBytes.push_back('\\');
					f_EncodedBytes.push_back('n');
					break;
				case '\t':  // Tab
					f_EncodedBytes.push_back('\\');
					f_EncodedBytes.push_back('t');
					break;
				case '\\':  // Backslash
					f_EncodedBytes.push_back('\\');
					f_EncodedBytes.push_back('\\');
					cout << "what the" << "\n";
					break;
				default:
					f_EncodedBytes.push_back(static_cast<uint8_t>(_c));
					break;
				}
			}

			Encode32BitInt(fp_ByteCode, static_cast<uint32_t>(f_EncodedBytes.size())); // Store the length of the string
			fp_ByteCode.insert(fp_ByteCode.end(), f_EncodedBytes.begin(), f_EncodedBytes.end());
		}

		void
			EncodeFloat(vector<uint8_t>&fp_ByteCode, float fp_Float)
		{
			uint32_t asInt;
			memcpy(&asInt, &fp_Float, sizeof(float)); // Copy the float into an uint32_t bit pattern
			Encode32BitInt(fp_ByteCode, asInt);      // Reuse the integer encoding function
		}

		void
			Encode32BitChar(vector<uint8_t>&fp_ByteCode, uint32_t character)
		{
			Encode32BitInt(fp_ByteCode, character); // Treat the character as a 32-bit integer
		}

		void
			EncodeBool(vector<uint8_t>&fp_ByteCode, bool fp_Bool)
		{
			uint32_t boolAsInt = fp_Bool ? 1 : 0; // Convert boolean to 32-bit integer
			Encode32BitInt(fp_ByteCode, boolAsInt);
		}

	public:

		//////////////////////////////////////////////
		// MAIN COMPILING FUNCTION
		//////////////////////////////////////////////

		bool //doesn't return anything because the compiler writes the output to a .bongo file
			CompileProgram
			(
				const string & fp_DesiredBongoScriptFilePath, 
				const string & fp_DesiredOutputDirectory, 
				const string & fp_DesiredOutputFileName, 
				Logger* logger,
				const bool fp_IsDebug = false
			)
		{
			//////////////////// Read .bj file, Tokenize and Parse it ////////////////////

			Parser f_BongoParser = Parser(); //needa make this a class since that's the only way cpp will let me do mutual recursion for some reason lmao

			string f_SourceCode;
			if (not ReadFileIntoString(&f_SourceCode, fp_DesiredBongoScriptFilePath, logger))
			{
				//stop compilation immediately
				logger->LogAndPrint("Compiler was not able to read a valid source file, compilation will not proceed any further. nothing was done.", "Compiler", Logger::LogLevel::Fatal);
				return false;
			}

			vector<Token> f_ProgramTokens;
			Tokenize(f_SourceCode, f_ProgramTokens, logger);

			Program* f_BongoProgram = f_BongoParser.ConstructAST(f_ProgramTokens);

			//////////////////// Check for Main Func ////////////////////

			vector<uint8_t> f_CompiledByteCode;
			unique_ptr<FuncDeclaration> f_MainFunc = move(f_BongoProgram->m_ProgramFunctions.back());

			if (f_MainFunc->m_FuncName.m_Value != "main")
			{
				//THROW ERROR
				logger->LogAndPrint("Compiler Error: main function is not defined as the last function in the program", "Compiler", Logger::LogLevel::Error);
				logger->LogAndPrint("Please take a look at your program structure and re-organize it such that main() is defined last", "Compiler", Logger::LogLevel::Warning);
				return false;
			}

			//////////////////// Define Main Loop Variables ////////////////////

			size_t f_ProgramCounter = 0; //idk why but ill keep track of where we are

			bool f_ShouldShift = true;

			unique_ptr<StatementNode> f_CurrentProgramStatement;

			//////////////////// Main Compile Loop ////////////////////

			//cout << CreateColouredText("MainFunc Token Size: ", "bright green") << static_cast<PrintFunction>(f_MainFunc->m_CodeBody[0]) << "\n";

			while (f_MainFunc->m_CodeBody.size() > 0) //compiling the main function code body
			{
				f_CurrentProgramStatement = ShiftForward(f_MainFunc->m_CodeBody);
				f_ProgramCounter++;

				switch (f_CurrentProgramStatement->m_Domain)
				{
				case SyntaxNodeType::PrintFunction:
				{
					f_CompiledByteCode.push_back(0x02A); //print function opcode
					f_CompiledByteCode.push_back(0x0AA); //STRING_LITERAL

					PrintFunction* _pf = dynamic_cast<PrintFunction*>(f_CurrentProgramStatement.get());
					string s_TextColour = "white";

					if (_pf->m_FuncArgs[1].size() > 0) //check if the colour arg exists
					{
						//f_CompiledByteCode.push_back(0x0CE); //COLOURIZE
						s_TextColour = (dynamic_cast<StringLiteral*>(_pf->m_FuncArgs[1][0].get()))->m_StringValue.m_Value; //only handling string literals for now
					}

					//////////////////// Parse First Argument ////////////////////

					// [0][_i] because the 0th index refers to the string input, the _i'th input is if we're doing string operations inside like "hello" + myVar + "world"
					for (int _i = 0; _i < _pf->m_FuncArgs[0].size(); _i++)
					{
						EncodeUTF8String
						(
							f_CompiledByteCode,
							CreateColouredText
							(
								(dynamic_cast<StringLiteral*>(_pf->m_FuncArgs[0][_i].get()))->m_StringValue.m_Value,
								Colours::BrightMagenta
							)
						);
					}

					continue;
				}
				break;
				default:
					break;
				}

			}

			//////////////////// Write the Compiled Byte Code to a File ////////////////////

			WriteBytecodeToFile(f_CompiledByteCode, fp_DesiredOutputDirectory, fp_DesiredOutputFileName, logger);
			f_CompiledByteCode.clear(); //dump the vector since the code has been written to a file hopefully >w<

			delete f_BongoProgram;
			f_BongoProgram = nullptr;

			return true;
		}
	};
}