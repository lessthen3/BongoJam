#pragma once

#include "Parser.h"
#include <fstream>

using namespace std;

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

	//////////////////////////////////////////////
	// Opcodes Lookup for Translation
	//////////////////////////////////////////////

	const map<const string, const uint8_t> OPCODES_LOOKUP = //these opcodes are flags that indicate to the interpreter what information is going to follow it in the byte stream
	{
		{"NULL", 0x00},

		{"INT", 0x01},
		{"FLOAT", 0x02},

		{"BOOL", 0x03},

		{"CHAR", 0x04},
		{"STRING", 0x05},

		{"VOID", 0x06},
		{"EXCEPTION", 0x07},

		{"THREAD", 0x07},

		{"VEC2", 0x016},
		{"VEC3", 0x017},
		{"VEC4", 0x00},

		{"MAT2", 0x018},
		{"MAT3", 0x019},
		{"MAT4", 0x01A},
		{"MAT", 0x01A},

		{"LOGICAL_AND", 0x00},
		{"LOGICAL_OR", 0x00},
		{"LOGICAL_NOT", 0x00},

		{"LINE_NUMBER", 0x05}, //Used for tracking the exact line of code that threw a runtime error

		{"JUMP", 0x06},
		{"JUMP_IF", 0x07},

		{"VARIABLE_REASSIGNMENT", 0x09},

		{"FUNCTION_DEFINITION", 0x0A},
		{"METHOD_DEFINITION", 0x0B},

		{"FUCTION_CALL", 0x0C},
		{"METHOD_CALL", 0x0D},

		{"FUNCTION_RETURN", 0x0D},

		{"CLASS_DEFINITION", 0x0E},
		{"STRUCT_DEFINITION", 0x0F},

		{"CLASS_CONSTRUCTOR", 0x010},
		{"STRUCT_CONSTRUCTOR", 0x011},

		{"ADD", 0x012},
		{"SUB", 0x013},

		{"DIV", 0x014},
		{"MULT", 0x015},

		// Follows a Variable Assignment flag

		{"LOAD", 0x016}, //stack allocates a var
		{"UNLOAD", 0x017}, //dereferences stack alloc'd var in interpreter
		{"MOVE", 0x018}, //move() semantic
		{"COPY", 0x07}, //used for copying vars
		{"HEAP_ALLOC", 0x019}, //used for heap allocations
		{"HEAP_DE_ALLOC", 0x01A}, //delete baby

		{"PRINT", 0x019},
		{"CLOCK", 0x01A},
		{"SLEEP", 0x01B},

		{"INPUT", 0x0FF},
		{"ROUND_UP", 0x01B},
		{"ROUND_DOWN", 0x01B},
		{"SQUARE_ROOT", 0x01B},
		{"POWER", 0x01B},
		{"EXP", 0x00},
		{"SIN", 0x01B},
		{"COS", 0x01B},
		{"SINH", 0x01B},
		{"COSH", 0x00},
		{"ARCSIN", 0x00},
		{"ARCCOS", 0x00},
	};

	//////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////

	static StatementNode
		ShiftForward(vector<StatementNode>& fp_ProgramBody)
	{
		if (fp_ProgramBody.empty())
		{
			return StatementNode(); //return escape char when source code is done being read
		}

		StatementNode f_FirstElement = fp_ProgramBody.front();
		fp_ProgramBody.erase(fp_ProgramBody.begin());

		return f_FirstElement;
	}

	void 
		WriteBytecodeToFile(const string& filename, const vector<uint8_t>& bytecode) 
	{
		ofstream file(filename + ".bongo", ios::binary);  // Open in binary mode

		if (!file) 
		{
			LogAndPrint("Failed to open the file for writing.", "Compiler", "error", "red");
			return;
		}

		// Write the entire contents of the vector to the file
		file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());

		file.close();  // Close the file
	}


	//////////////////////////////////////////////
	// Encoding Functions
	//////////////////////////////////////////////

	static void
		Encode32BitInt(vector<uint8_t>& fp_ByteCode, uint32_t fp_Int)
	{
		fp_ByteCode.push_back((fp_Int >> 24) & 0xFF); // High byte
		fp_ByteCode.push_back((fp_Int >> 16) & 0xFF);
		fp_ByteCode.push_back((fp_Int >> 8) & 0xFF);
		fp_ByteCode.push_back(fp_Int & 0xFF);         // Low byte
	}

	static void
		EncodeUTF8String(vector<uint8_t>& bytes, const string& str)
	{
		Encode32BitInt(bytes, static_cast<uint32_t>(str.size())); // Store the length of the string

		for (char _c : str)
		{
			bytes.push_back(static_cast<uint8_t>(_c)); // Store each character
		}
	}

	static void
		EncodeFloat(vector<uint8_t>& fp_ByteCode, float fp_Float)
	{
		uint32_t asInt;
		memcpy(&asInt, &fp_Float, sizeof(float)); // Copy the float into an uint32_t bit pattern
		Encode32BitInt(fp_ByteCode, asInt);      // Reuse the integer encoding function
	}

	static void
		Encode32BitChar(vector<uint8_t>& fp_ByteCode, uint32_t character)
	{
		Encode32BitInt(fp_ByteCode, character); // Treat the character as a 32-bit integer
	}

	static void
		EncodeBool(vector<uint8_t>& fp_ByteCode, bool fp_Bool)
	{
		uint32_t boolAsInt = fp_Bool ? 1 : 0; // Convert boolean to 32-bit integer
		Encode32BitInt(fp_ByteCode, boolAsInt);
	}
	
	//////////////////////////////////////////////
	// Compile to Byte-Code
	//////////////////////////////////////////////

	static void //doesn't return anything because the compiler writes the output to a .bongo file
		CompileProgram(Program& fp_Program)
	{
		vector<uint8_t> f_CompiledByteCode;

		size_t f_ProgramCounter = 0;

		bool f_ShouldShift = true;

		while (fp_Program.m_ProgramBody.size() > 0)
		{



		}

		//////////////////// Write the Compiled Byte Code to a File ////////////////////

		WriteBytecodeToFile("UwU", f_CompiledByteCode);
		f_CompiledByteCode.clear(); //dump the vector since the code has been written to a file hopefully >w<
	}
}