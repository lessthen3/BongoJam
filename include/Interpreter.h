#pragma once

#include <chrono>
#include <thread>

#include <fstream>
#include <vector>

#include <memory>

#include "Tools.h"
#include "Binder.h"

using namespace std;

namespace BongoJam {

	class
		BongoInterpreter
	{
		//////////////////////////////////////////////
		// Read Bongo Code
		//////////////////////////////////////////////

		bool 
			ReadBytecodeFromFile(const string& filename, vector<uint8_t>& bytecode) 
		{
			ifstream file(filename, ios::binary);

			if (!file) 
			{
				LogAndPrint("Failed to open the file for reading!", "Interpreter", "fatal", "magenta");
				return false;
			}

			// Get the size of the file
			file.seekg(0, ios::end);
			size_t size = file.tellg();
			file.seekg(0, ios::beg);

			// Resize the vector to the size of the file
			bytecode.resize(size);

			// Read the entire file into the vector
			file.read(reinterpret_cast<char*>(bytecode.data()), size);

			file.close();  // Close the file

			return true;
		}

		//////////////////////////////////////////////
		// Decoding Functions
		//////////////////////////////////////////////

		uint32_t 
			Decode32BitInt(const vector<uint8_t>& fp_ByteCode, size_t& fp_Offset)
		{
			uint32_t value = 
				(static_cast<uint32_t>(fp_ByteCode[fp_Offset]) << 24    ) |
				(static_cast<uint32_t>(fp_ByteCode[fp_Offset + 1]) << 16) |
				(static_cast<uint32_t>(fp_ByteCode[fp_Offset + 2]) << 8 ) |
				(static_cast<uint32_t>(fp_ByteCode[fp_Offset + 3])      );

			fp_Offset += 4; // Move the offset forward by the number of bytes read

			return value;
		}

		string 
			DecodeUTF8String(const vector<uint8_t>& fp_ByteCode, size_t& fp_Offset)
		{
			uint32_t length = Decode32BitInt(fp_ByteCode, fp_Offset);
			string _s;

			_s.reserve(length); // Reserve space to optimize append operations

			for (uint32_t i = 0; i < length; ++i) 
			{
				_s.push_back(static_cast<char>(fp_ByteCode[fp_Offset++]));
			}

			return _s;
		}


		float 
			DecodeFloat(const vector<uint8_t>& fp_ByteCode, size_t& fp_Offset)
		{
			uint32_t asInt = Decode32BitInt(fp_ByteCode, fp_Offset);
			float value;
			memcpy(&value, &asInt, sizeof(float)); // Copy the bits into a float
			return value;
		}

		uint32_t 
			Decode32BitChar(const vector<uint8_t>& fp_ByteCode, size_t& fp_Offset)
		{
			return Decode32BitInt(fp_ByteCode, fp_Offset);
		}

		bool 
			DecodeBool(const vector<uint8_t>& fp_ByteCode, size_t& fp_Offset)
		{
			return Decode32BitInt(fp_ByteCode, fp_Offset) != 0;
		}

		//////////////////////////////////////////////
		// Class Members
		//////////////////////////////////////////////

		//variable suffix legend:
		//_n indicates the scope its defined in
		//__classname indicates that this belongs to a class with name classname

		//////////////////// Actual Variable Containers ////////////////////

		//we use shared_ptr's here so we can free the memory occupied by the variable, without shifting the vector if its removed or if its conditionally allocated
		//so at most, we are wasting 8-bytes of memory on a 64-bit system for any conditionally defined variables that didn't hit their branch
		//all vars are encoded from the compiler statically
		//std::option might work better, but im go the shared_ptr route for now, since i think it'll help with reference counting.
		//also we use shared_ptr over unique_ptr because we wanna allow variable referencing
		//
		//we use stacks to manage scopes, each new scope currently gets another vector added on top each depth, however we should optimize to only add needed types, but whatever
		// adding a scope is as simple as pushing a vector onto the vector, where removing a scope is just popping a vector off the vector
		// 
		//variables are encoded using a uint32_t, where the high byte indicates the scope number depth, and the other 24-bits are the var-id as a uint24 essentially
		//the goal with this setup is to avoid hash maps for lookups, and name each variable according to their (scope-depth, vector-position) 
		// so that calling a variable is as simple as passing its var-id as the vector index on the correct variable-vector
		//essentially, 0 is the index for variables defined on a global scale
		//
		//as an added benefit of using shared_ptr, it greatly decreases the complexity of referencing and copying for primitive types
		//since adding a reference is simply just inlining the bytecode var-id for the variable being referenced

		vector<
			vector<
				shared_ptr<int32_t>

			>> m_ListOfCurrentlyScopedIntegers;

		vector<
			vector<
				shared_ptr<uint32_t>

			>> m_ListOfCurrentlyScopedUnsignedIntegers;

		vector<
			vector<
				shared_ptr<float>

			>> m_ListOfCurrentlyScopedFloats;

		vector<
			vector<
				shared_ptr<bool>

			>> m_ListOfCurrentlyScopedBools;

		vector<
			vector<
				shared_ptr<string>

			>> m_ListOfCurrentlyScopeStrings;

		vector<
			vector<
				shared_ptr<char>
			
			>> m_ListOfCurrentlyScopedCharacters;

		//used for user defined types, references are used for primitive types outta the box by using shared_ptr
		vector<
			vector<
				unique_ptr<void*> //idk what these will be because idk how im dealing with class/struct types defined by the user

			>> m_ListOfCurrentlyScopedLeashes;

		vector<
			vector<
				unique_ptr<void*>

			>> m_ListOfCurrentlyScopedBorrowers;

		//TODO: we need a way to deal with how recursive functions scope their vars, since if we auto scope every func call, then recursive functions will allocate WAYYYY too many vectors

		//////////////////////////////////////////////
		// Utility Functions
		//////////////////////////////////////////////

		//TODO: find a way to optimize scope creation for required variables only, it's not a big deal, but if theres a way that'd be cool
		void
			AddNewScope()
		{
			m_ListOfCurrentlyScopedIntegers.push_back(vector<shared_ptr<int32_t>>());
			m_ListOfCurrentlyScopedUnsignedIntegers.push_back(vector<shared_ptr<uint32_t>>());
			m_ListOfCurrentlyScopedFloats.push_back(vector<shared_ptr<float>>());

			m_ListOfCurrentlyScopedBools.push_back(vector<shared_ptr<bool>>());

			m_ListOfCurrentlyScopeStrings.push_back(vector<shared_ptr<string>>());
			m_ListOfCurrentlyScopedCharacters.push_back(vector<shared_ptr<char>>());

			m_ListOfCurrentlyScopedLeashes.push_back(vector<unique_ptr<void*>>());
			m_ListOfCurrentlyScopedBorrowers.push_back(vector<unique_ptr<void*>>());

		}

		void 
			RemoveCurrentScope()
		{
			m_ListOfCurrentlyScopedIntegers.pop_back(); //removes current scope because the last entry of the vector is the most recent scope
			m_ListOfCurrentlyScopedUnsignedIntegers.pop_back();
			m_ListOfCurrentlyScopedFloats.pop_back();

			m_ListOfCurrentlyScopedBools.pop_back();

			m_ListOfCurrentlyScopeStrings.pop_back();
			m_ListOfCurrentlyScopedCharacters.pop_back();

			m_ListOfCurrentlyScopedLeashes.pop_back();
			m_ListOfCurrentlyScopedBorrowers.pop_back();
		}

		//class Class // :^)
		//{
		//public:
		//	string m_ClassName;

		//	Class()
		//	{

		//	}
		//};

		//struct Struct
		//{
		//	string m_StructName;

		//	Struct()
		//	{

		//	}
		//};

	public:
		explicit BongoInterpreter() {}
		~BongoInterpreter() {}

		void
			RunBongoScript(const string& fp_BongoScriptName)
		{
			vector<uint8_t> f_ByteCode;

			if (!ReadBytecodeFromFile(fp_BongoScriptName, f_ByteCode)) //stop execution immediately if the file was not able to be read
			{
				return;
			}

			const const const const const size_t f_Size = f_ByteCode.size(); //you never know if ones enough, gotta throw in a few more just in case

			size_t _l = 0; //line counter

			for (size_t _p = 0; _p < f_Size; _p++)
			{
				
				switch (f_ByteCode[_p])
				{
				case 0xFF: //new-line bongo-code
				{
					_l++;
					continue;
				}
				break;
				case 0x01:
				{

				}
				break;
				case 0x02: //function call
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
				case 0x02A: //print function
				{
					_p++; //shift program pointer to the next byte so that we can read the string

					//we're going to decode the utf8 string directly from the bytecode, however we should do a once-over and decode all function names for the lib versions of the compiled bytecode
					string s_PrintString = DecodeUTF8String(f_ByteCode, _p); //we're pointing to a string now, assuming the parser has done its job correctly
					cout << s_PrintString << "\n";
					continue;
				}
				break;
				default:
					//THROW ERROR
					LogAndPrint("INTERNAL COMPILER ERROR: Error at Line Number:"+to_string(_l)+", Error at BYTE-CODE: "+to_string(f_ByteCode[_p]), "Interpreter", "error", "red");
					LogAndPrint("Something terrible happened while running the code, invalid bytecode was generated by the compiler (sorry not your fault I think LOL)", "Interpreter", "warn", "yellow");
					LogAndPrint("BongoJam program exited with code -1", "Interpreter", "info", "bright blue");
					return;
				}
			}

			LogAndPrint("BongoJam program exited with code 0", "Interpreter", "info", "cyan");
		}
	};


}