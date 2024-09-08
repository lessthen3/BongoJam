/*****************************************************************
 *                                        BongoJam Script v0.3
 *                           Created by Ranyodh Mandur - © 2024
 *
 *                         Licensed under the MIT License (MIT).
 *                  For more details, see the LICENSE file or visit:
 *					https://opensource.org/licenses/MIT
 *
 *		BongoJam is an open-source scripting language compiler and interpreter
 *				primarily intended for embedding within game engines.
 *****************************************************************/
#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <map>

#include <algorithm>
#include <cctype>

#include "Tools.h"

using namespace std;

namespace BongoJam {

	//////////////////////////////////////////////
	// Token and Token-type Definition
	//////////////////////////////////////////////

	enum class TokenType
	{
		//////////////////// GOATS ////////////////////

		IntNumber,
		FloatNumber,
		UserIdentifier,
		Import,

		//////////////////// Operators ////////////////////

		Equals,
		StrictlyEquals,
		DoesNotEquals,

		GreaterThan,
		LesserThan,
		GreaterThanOrEqual,
		LesserThanOrEqual,

		AdditionOperator,
		NegativeOperator, //don't need subtraction

		MultiplicationOperator,
		DivisionOperator,

		ModulusOperator,

		PlusEqualsOperator,
		MinusEqualsOperator,
		MultEqualsOperator,
		DivEqualsOperator,

		ModuloEqualsOperator,

		//////////////////// Bracket Types ////////////////////

		OpenParen,
		CloseParen,

		OpenBracket,
		CloseBracket,

		OpenSquareBracket,
		CloseSquareBracket,

		//////////////////// Symbols ////////////////////

		DoubleDot,
		SemiDot,
		Dot,
		Comma,

		DollarSign,
		AtSign,
		QuestionMark,
		QuotationMark,
		HashTag,

		//////////////////// Declaratives ////////////////////

		Let,
		Func,
		Return,
		Class,
		Struct,
		Enum,
		Interface,

		//////////////////// Conditional Logic ////////////////////

		If,
		Elif,
		Else,
		For,
		ForEach,
		While,
		Match,
		Break,
		Continue,
		Try,
		Catch,

		//////////////////// Boolean Operators ////////////////////

		And,
		Or,
		Not,
		Is,

		//////////////////// Modifiers ////////////////////

		Const,
		Static,
		Public,
		Protected,
		Private, //used for tracking later on in the parser and interpreter
		As,
		In,
		Extends,
		Single,
		Template, //used for defining abstract class'

		Parent,
		This,
		Big, //used as the enlargement modifier for container sizing of floats and ints

		New,
		Delete,

		//////////////////// Error Handling ////////////////////

		Panic,

		//////////////////// Types ////////////////////

		TypeArrow,
		Null,
		Void,
		Exception,

		Int,
		Float,
		Bool,
		True,
		False,
		String,
		StringLiteral,
		Any,

		List,
		Dictionary,

		Thread,

		Event,
		Leash, //any var can have a manager, but only one at a time. you can have managers borrow clients from other managers temporarily, but the lifetime must not exceed the length of the original manager because ultimately the lifetime of the client is still tied to the manager
		Borrower,
		Reader,

		Vector2,
		Vector3,
		Vector,

		Mat2,
		Mat3,
		Mat4,
		Mat,

		//////////////////// Included Functions ////////////////////

		Print,
		Input,
		Colourize,

		Clock,
		TypeOf,
		Cast,
		Length,

		Floor,
		Ceiling,

		SquareRoot,
		Power,

		Sin,
		Cos,
		Tan,

		ArcSin,
		ArcCos,
		ArcTan,

		Sinh,
		Cosh,
		Tanh,

		Exp,
		NaturalLog,
		Factorial,

		NO_TOKEN_VALUE,
		ENDF
	};
	
	struct Token
	{
		string m_Value;
		TokenType m_Type;
		int m_SourceCodeLineNumber;

		explicit Token(const string& fp_Value, const TokenType fp_Type, const int fp_SourceCodeLineNumber)
		{
			m_Value = fp_Value;
			m_Type = fp_Type;
			m_SourceCodeLineNumber = fp_SourceCodeLineNumber;
		}

		explicit Token(const char& fp_Value, const TokenType fp_Type, const int fp_SourceCodeLineNumber)
		{
			m_Value = fp_Value;
			m_Type = fp_Type;
			m_SourceCodeLineNumber = fp_SourceCodeLineNumber;
		}

		explicit Token(): m_Value(""), m_Type(TokenType::NO_TOKEN_VALUE), m_SourceCodeLineNumber(-1) {}
	};

	//////////////////////////////////////////////
	// Keywords Definition
	//////////////////////////////////////////////

	const map<string, TokenType> KEYWORDS =
	{
		//////////////////// GOAT ////////////////////

		{"import", TokenType::Import}, //PLEASE I LOVE USING OTHER PEOPLE'S CODE

		//////////////////// Declaratives ////////////////////

		{"let", TokenType::Let},
		{"func", TokenType::Func},
		{"return", TokenType::Return},
		{"class", TokenType::Class},
		{"struct", TokenType::Struct},
		{"enum", TokenType::Enum},
		{"interface", TokenType::Interface},

		//////////////////// Conditional Logic ////////////////////

		{"if", TokenType::If},
		{"elif", TokenType::Elif},
		{"else", TokenType::Else},
		{"for", TokenType::For},
		{"foreach", TokenType::ForEach},
		{"while", TokenType::While},
		{"match", TokenType::Match},
		{"break", TokenType::Break},
		{"continue", TokenType::Continue},
		{"try", TokenType::Try},
		{"catch", TokenType::Catch},

		//////////////////// Boolean Operators ////////////////////

		{"and", TokenType::And},
		{"or", TokenType::Or},
		{"not", TokenType::Not},
		{"is", TokenType::Is},

		//////////////////// Modifiers ////////////////////

		{"const", TokenType::Const},
		{"static", TokenType::Static},
		{"public", TokenType::Public}, //private is implied
		{"protected", TokenType::Protected},
		{"as", TokenType::As},
		{"in", TokenType::In},
		{"extends", TokenType::Extends},
		{"single", TokenType::Single},
		{"template", TokenType::Template}, //func's or class' or var's can be templated, this is the replacement for virtual.

		{"parent", TokenType::Parent}, //used as the stand-in for super, because super is a retarded name for the keyword
		{"this", TokenType::This},
		{"big", TokenType::Big}, //used for expanding float or int container size to 64 or 128 bits

		{"new", TokenType::New}, //used for heap-allocations
		{"delete", TokenType::Delete}, //used for clearing heap-allocations

		//////////////////// Error Handling ////////////////////
		
		{"panic", TokenType::Panic},

		//////////////////// Types ////////////////////

		{"null", TokenType::Null},
		{"void", TokenType::Void},
		{"exception", TokenType::Exception},

		{"int", TokenType::Int},
		{"decimal", TokenType::Float},
		{"bool", TokenType::Bool},
		{"true", TokenType::True},
		{"false", TokenType::False},
		{"text", TokenType::String},
		{"any", TokenType::Any},

		{"List", TokenType::List},
		{"Dictionary", TokenType::Dictionary},

		{"Thread", TokenType::Thread},

		{"event", TokenType::Event},
		{"leash", TokenType::Leash}, //unique_ptr
		{"borrower", TokenType::Borrower}, //move() on a unique_ptr
		{"reader", TokenType::Reader}, //readonly ptr

		{"Vector2", TokenType::Vector2},
		{"Vector3", TokenType::Vector3},
		{"Vector", TokenType::Vector}, //generic n-length vector

		{"Mat2", TokenType::Mat2},
		{"Mat3", TokenType::Mat3},
		{"Mat4", TokenType::Mat4},
		{"Mat", TokenType::Mat}, //generic nxm matrix

		//////////////////// Included Functions ////////////////////

		{"print", TokenType::Print},
		{"input", TokenType::Input},
		{"colourize", TokenType::Colourize},

		{"clock", TokenType::Clock},
		{"typeof", TokenType::TypeOf},
		{"cast", TokenType::Cast},
		{"len", TokenType::Length},

		// math functions
		{"round_down", TokenType::Floor},
		{"round_up", TokenType::Ceiling},

		{"sqrt", TokenType::SquareRoot},
		{"pow", TokenType::Power},

		{"sin", TokenType::Sin},
		{"cos", TokenType::Cos},
		{"tan", TokenType::Tan},

		{"cosh", TokenType::Cosh},
		{"sinh", TokenType::Sinh},
		{"tanh", TokenType::Tanh},

		{"arccos", TokenType::ArcCos},
		{"arcsin", TokenType::ArcSin},
		{"arctan", TokenType::ArcTan},

		{"exp", TokenType::Exp},
		{"log", TokenType::NaturalLog},
		{"factorial", TokenType::Factorial}
	};

	//////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////

	// Helper function to Trim whitespace from both ends of a string
	string Trim(const string& str)
	{
		auto start = str.begin();

		while (start != str.end() && isspace(*start))
		{
			start++;
		}
		auto end = str.end();

		do
		{
			end--;
		} while (distance(start, end) > 0 && isspace(*end));

		return string(start, end + 1);
	}

	vector<string> Split(const string& s, const vector<string>& delimiters)
	{
		vector<string> tokens;
		string temp = s;

		while (!temp.empty())
		{
			size_t minPosition = string::npos;
			string foundDelimiter;

			for (const auto& delimiter : delimiters)
			{
				size_t pos = temp.find(delimiter);

				if (pos < minPosition)
				{
					minPosition = pos;
					foundDelimiter = delimiter;
				}
			}

			if (minPosition != string::npos)
			{
				string token = temp.substr(0, minPosition);
				tokens.push_back(Trim(token)); // Trim whitespace and add the token
				temp.erase(0, minPosition + foundDelimiter.length());
			}
			else
			{
				tokens.push_back(Trim(temp)); // Last token after the final delimiter
				temp.clear();
			}
		}

		return tokens;
	}

	char ShiftForward(string& fp_Src)
	{
		if (fp_Src.empty())
		{
			return '\0';
		}

		char _c = fp_Src[0];
		fp_Src.erase(fp_Src.begin());
		return _c;
	}

	//////////////////////////////////////////////
	// Tokenize Function
	//////////////////////////////////////////////

	vector<Token> Tokenize(string& fp_SourceCode)
	{
		vector<Token> f_Tokens;

		size_t f_ProgramCounter = 0;
		size_t f_CurrentLineNumber = 1;

		char f_CurrentChar;

		bool f_ShouldShift = true;
		bool f_IsCurrentlyInsideComment = false;

		while(fp_SourceCode.size() > 0)
		{
			//////////////////// Iterate Current Character ////////////////////

			if(f_ShouldShift)
			{
				f_CurrentChar = ShiftForward(fp_SourceCode);
				f_ProgramCounter++;
			}
			else 
			{
				f_ShouldShift = true; //reset , only triggered for once loop iteration since while loops always step one character over their functioning bounds
			}

			//////////////////// Handle Spaces, New-Lines, and Comments ////////////////////

			if (f_CurrentChar == '\n') //used to keep track of what line number we're at in the source code, we only have single line comments, so this is sufficient
			{
				f_IsCurrentlyInsideComment = false;
				f_CurrentLineNumber++;
				continue; //we can shift forwards confidently since we're currently on the newline character
			}
			else if (f_IsCurrentlyInsideComment || isspace(f_CurrentChar))
			{
				continue;
			}

			//////////////////// Handle Digits or Alphabetic Characters ////////////////////

			if (isdigit(f_CurrentChar))
			{
				string f_Number = ""; // >w<

				while (fp_SourceCode.size() > 0 && isdigit(f_CurrentChar))
				{
					f_Number += f_CurrentChar;
					f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
					f_ProgramCounter++;
				}

				if (f_CurrentChar == '.') //used for handling decimal numbers eg. "let x->float = 3.14;"
				{
					f_Number += f_CurrentChar; //add the decimal so we're at: "69. (rest to be parsed)" currently
					f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
					f_ProgramCounter++;

					if (!isdigit(f_CurrentChar))
					{
						// Handle error: Unterminated type arrow
						LogAndPrint("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer", "error", "red");
						LogAndPrint("Unexpected symbol following a '.' brother!, looks like you've input a non-numeric symbol while defining a decimal number", "Lexer", "warn", "yellow");

						fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
						continue;
					}

					while (fp_SourceCode.size() > 0 && isdigit(f_CurrentChar))
					{
						f_Number += f_CurrentChar;
						f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
						f_ProgramCounter++;
					}
					//push a float
					f_Tokens.push_back(Token(f_Number, TokenType::FloatNumber, f_CurrentLineNumber)); //No need for a continue here since the current character isnt a digit
				}
				else
				{	//push an int
					f_Tokens.push_back(Token(f_Number, TokenType::IntNumber, f_CurrentLineNumber)); //No need for a continue here since the current character isnt a digit
				}

				f_ShouldShift = false; //ensures we don't skip any crucial branch-logic for the over-stepped character
				continue; //move to next iteration
			}

			else if (isalpha(f_CurrentChar) || f_CurrentChar == '_')
			{
				string f_Identifier = ""; //start with NOTHING

				while (fp_SourceCode.size() > 0 && (isalpha(f_CurrentChar) || f_CurrentChar == '_'))
				{
					f_Identifier += f_CurrentChar;
					f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
					f_ProgramCounter++;
				}

				if (KEYWORDS.find(f_Identifier) == KEYWORDS.end())
				{
					f_Tokens.push_back(Token(f_Identifier, TokenType::UserIdentifier, f_CurrentLineNumber));
				}
				else 
				{
					f_Tokens.push_back(Token(f_Identifier, KEYWORDS.at(f_Identifier), f_CurrentLineNumber));
				}

				f_ShouldShift = false; //ensures we don't skip any crucial branch-logic for the over-stepped character
				continue; //move to next iteration since
			}

			switch(f_CurrentChar)
			{
				case ';':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::SemiDot, f_CurrentLineNumber));
					break;
				case ':':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::DoubleDot, f_CurrentLineNumber));
					break;
				case '.':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::Dot, f_CurrentLineNumber));
					break;
				case ',':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::Comma, f_CurrentLineNumber));
					break;

				case '(':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::OpenParen, f_CurrentLineNumber));
					break;
				case ')':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::CloseParen, f_CurrentLineNumber));
					break;

				case '{':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::OpenBracket, f_CurrentLineNumber));
					break;
				case '}':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::CloseBracket, f_CurrentLineNumber));
					break;

				case '[':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::OpenSquareBracket, f_CurrentLineNumber));
					break;
				case ']':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::CloseSquareBracket, f_CurrentLineNumber));
					break;

				case '"': //VERY IMPORTANT THAT WE PROCESS THIS BEFORE '/' otherwise '/' mentioned inside of strings might be ignored
				{
					string f_CurrentStringLiteral = "";

					// Shift to the next character to start capturing the string, not the opening quote
					f_CurrentChar = ShiftForward(fp_SourceCode);
					f_ProgramCounter++;

					bool f_IsEscapeCharacter = false;

					while (fp_SourceCode.size() > 0 and f_CurrentChar != '"')
					{
						if (f_CurrentChar == '\\')
						{
							f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
							f_ProgramCounter++;

							switch (f_CurrentChar) 
							{
							case 'n':
								f_CurrentStringLiteral += '\n'; // Add a newline character
								break;
							case 't':
								f_CurrentStringLiteral += '\t'; // Add a tab character
								break;
							case '\\':
								f_CurrentStringLiteral += '\\'; // Add a literal backslash
								break;
							case '"':
								f_CurrentStringLiteral += '"'; // Add a literal double quote
								break;
							default:
								// Handle unknown escape sequences or add a fallback behavior
								f_CurrentStringLiteral += '\\'; // Re-add the backslash as it was part of the input
								f_CurrentStringLiteral += f_CurrentChar; // Add the unknown character as is
								break;
							}
						}
						else
						{
							f_CurrentStringLiteral += f_CurrentChar;
						}

						f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
						f_ProgramCounter++;
					}

					// Check if we've ended on the closing quotation mark
					if (f_CurrentChar == '"')
					{
						// Push the final string token without the quotes
						f_Tokens.push_back(Token(f_CurrentStringLiteral, TokenType::StringLiteral, f_CurrentLineNumber));

						// Shift again to move past the closing quote
						f_CurrentChar = ShiftForward(fp_SourceCode);
						f_ProgramCounter++;
					}
					else
					{
						// Handle error: Unterminated string literal, and exit program execution
						LogAndPrint("Unterminated string literal, brother! Error occured at line number: " + to_string(f_CurrentLineNumber), "Lexer", "error", "red");
						fp_SourceCode.clear();
						continue;
					}

					//IMPORTANT IF WE DONT MAKE SURE WE AVOID SHIFTING THEN EVERYTHING WILL BREAK
					f_ShouldShift = false; //just in case
					continue; //move to next iteration, we shift here because f_CurrentChar is pointing -> ' " ' 
				}
					break;
				case '=':
				{
					string f_EqualsString = "";

					while (fp_SourceCode.size() > 0 && f_CurrentChar == '=')
					{
						f_EqualsString += f_CurrentChar;
						f_CurrentChar = ShiftForward(fp_SourceCode);
						f_ProgramCounter++;
					}

					if (f_EqualsString.size() >= 2)
					{
						f_Tokens.push_back(Token(f_EqualsString, TokenType::StrictlyEquals, f_CurrentLineNumber));
					}
					else if (f_EqualsString.size() == 1)
					{
						f_Tokens.push_back(Token(f_EqualsString, TokenType::Equals, f_CurrentLineNumber));
					}
					else
					{
						//THROW ERROR
						LogAndPrint("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer", "error", "red");
						LogAndPrint("Something bad happened involving a '=' sign brother", "Lexer", "warn", "yellow");
						fp_SourceCode.clear(); //dump source code so that lexical analysis ends immediately
					}

					f_ShouldShift = false; //as usual, skip next iteration since we end on the character right after the last '=' char
					continue; //continue will just move the current over-stepped character back to the top of the lexer's logical flow
				}
				break;
				case '!':
				{
					string s_DoesNotEqualsString = "!";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode);
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						s_DoesNotEqualsString += f_CurrentChar;
						f_Tokens.push_back(Token(s_DoesNotEqualsString, TokenType::DoesNotEquals, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else //handles the case for when nothing valid follows a '!' in the source code
					{
						//THROW ERROR
						LogAndPrint("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer", "error", "red");
						LogAndPrint("Something bad happened involving a '!' sign brother", "Lexer", "warn", "yellow");
						fp_SourceCode.clear(); //dump source code so that lexical analysis ends immediately
						continue;
					}
				}
				break;
				case '+':
				{
					string f_PlusString = "+";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						f_PlusString += f_CurrentChar;
						f_Tokens.push_back(Token(f_PlusString, TokenType::PlusEqualsOperator, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else //handles the case for when nothing valid follows a '!' in the source code
					{
						f_Tokens.push_back(Token(f_PlusString, TokenType::AdditionOperator, f_CurrentLineNumber));
						f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
						continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
					}
				}
					break;
				case '*':
				{
					string f_MultString = "*";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						f_MultString += f_CurrentChar;
						f_Tokens.push_back(Token(f_MultString, TokenType::MultEqualsOperator, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else //handles the case for when nothing valid follows a '!' in the source code
					{
						f_Tokens.push_back(Token(f_MultString, TokenType::MultiplicationOperator, f_CurrentLineNumber));
						f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
						continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
					}
				}
					break;
				case '-':
				{
					string f_TypeArrow = ""; //ik whats coming next, but for conventions sake

					while (fp_SourceCode.size() > 0 && f_CurrentChar == '-')
					{
						f_TypeArrow += f_CurrentChar;
						f_CurrentChar = ShiftForward(fp_SourceCode);
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '>') //this goes first so that subtraction doesn't get confused with '->', since they both have only 1 dash
					{
						f_TypeArrow += f_CurrentChar;
						f_Tokens.push_back(Token(f_TypeArrow, TokenType::TypeArrow, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else if (f_CurrentChar == '=')
					{
						f_TypeArrow += f_CurrentChar; //IDC THAT ITS NOT A TYPEARROW
						f_Tokens.push_back(Token(f_TypeArrow, TokenType::MinusEqualsOperator, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else if (f_TypeArrow.size() == 1) //fuck it we ball, we deal with minus here BROTHERS
					{
						f_Tokens.push_back(Token(f_TypeArrow, TokenType::NegativeOperator, f_CurrentLineNumber));
						f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
						continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
					}
					else //if we have more than one consecutive '-', then it's a mistake regardless of what you were trying to do
					{
						// Handle error: Unterminated type arrow
						LogAndPrint("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer", "error", "red");
						LogAndPrint("Unterminated type arrow brother!, looks like you're missing an arrow head to your type arrow definition", "Lexer", "warn", "yellow");

						fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
						continue;
					}
				}
					break;
				case '/':
				{
					string f_DivString = "/";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						f_DivString += f_CurrentChar;
						f_Tokens.push_back(Token(f_DivString, TokenType::DivEqualsOperator, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else if (f_CurrentChar == '/')
					{
						f_IsCurrentlyInsideComment = true;
						continue;
					}
					else //handles the case for when nothing valid follows a '!' in the source code
					{
						f_Tokens.push_back(Token(f_DivString, TokenType::DivisionOperator, f_CurrentLineNumber));
						f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
						continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
					}
				}
					break;
				case '%':
				{
					string f_ModString = "%";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						f_ModString += f_CurrentChar;
						f_Tokens.push_back(Token(f_ModString, TokenType::ModuloEqualsOperator, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else //handles the case for when nothing valid follows a '!' in the source code
					{
						f_Tokens.push_back(Token(f_ModString, TokenType::ModulusOperator, f_CurrentLineNumber));
						f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
						continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
					}
				}
					break;

				case '<': //idrc about inserting this into the branch above with '>'
				{
					string f_LesserThanString = "<";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode);
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						f_LesserThanString += f_CurrentChar;
						f_Tokens.push_back(Token(f_LesserThanString, TokenType::LesserThanOrEqual, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else //handles the case for just '<'
					{
						f_Tokens.push_back(Token(f_LesserThanString, TokenType::LesserThan, f_CurrentLineNumber));
						f_ShouldShift = false; //iterate without shifting since we've overstepped a character
						continue;
					}
				}
				break;
				case '>':
				{
					string f_GreaterThanString = ">";

					if (fp_SourceCode.size() > 0) //make sure we're not going out of bounds before shifting xd
					{
						f_CurrentChar = ShiftForward(fp_SourceCode);
						f_ProgramCounter++;
					}

					if (f_CurrentChar == '=')
					{
						f_GreaterThanString += f_CurrentChar;
						f_Tokens.push_back(Token(f_GreaterThanString, TokenType::GreaterThanOrEqual, f_CurrentLineNumber));
						assert(f_ShouldShift == true);
						continue; //just iterate as normal, and make sure the equals character isn't double counted
					}
					else //handles the case for just '>'
					{
						f_Tokens.push_back(Token(f_GreaterThanString, TokenType::GreaterThan, f_CurrentLineNumber));
						f_ShouldShift = false; //iterate without shifting since we've overstepped a character
						continue;
					}
				}
				break;
				case '$':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::DollarSign, f_CurrentLineNumber));
					break;
				case '?':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::QuestionMark, f_CurrentLineNumber));
					break;
				case '@':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::AtSign, f_CurrentLineNumber));
					break;
				case '#':
					f_Tokens.push_back(Token(f_CurrentChar, TokenType::HashTag, f_CurrentLineNumber));
					break;
				default:

					LogAndPrint("Compiler Error: Unrecognized character found in source code at line " + to_string(f_CurrentLineNumber), "Lexer", "error", "red");

					fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
					break;
			}

		}

		f_Tokens.push_back(Token("", TokenType::ENDF, f_CurrentLineNumber)); //label the end of the file i guess for some reason
		return f_Tokens;
	}
}