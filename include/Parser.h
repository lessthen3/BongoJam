#pragma once

#include <cstring> // for memcpy

#include "AST.h"
#include "Lexer.h"

namespace BongoJam {

	//////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////

	static bool 
		IsENDF(Token fp_Token)
	{
		if (fp_Token.m_Type == TokenType::ENDF)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static Token
		ShiftForward(vector<Token>& fp_TokenVector)
	{
		if (fp_TokenVector.empty())
		{
			return Token("\0", TokenType::ENDF, -1); //return escape char when source code is done being read
		}

		Token f_FirstElement = fp_TokenVector.front();
		fp_TokenVector.erase(fp_TokenVector.begin());
		return f_FirstElement;
	}

	static bool 
		FindStringInVector(const vector<string>& fp_Vector, const string& fp_DesiredString) 
	{
		// Use find to search for the string
		auto _S = find(fp_Vector.begin(), fp_Vector.end(), fp_DesiredString);

		// Check if the string was found
		if (_S != fp_Vector.end()) 
		{
			// Found the string
			return true;
		}
		// String not found
		return false;
	}

	//////////////////////////////////////////////
	// Parsing Functions
	//////////////////////////////////////////////
	//Important: all parse functions call ShiftForward() first thing because the functions are assuming the context they are being called in.
	//that context being: you"ve found the corresponding indicator token, and the "stack pointer" aka f_CurrentToken is still sitting on that indicator token
	//eg. im parsing and find a "class" token, when I call ParseClassDeclaration(), the f_CurrentToken i pass is f_CurrentToken = "class" token.

	static bool
		ParseRegularExpr() //used for parsing any expr, and types it accordingly
	{

	}

	//this method will find the smallest possible expression formed by the tokens following the number token, and return it
	static bool
		ParseNumber(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, Expr* fp_NumberExpr)
	{


		return true;
	}

	//////////////////////////////////////////////
	// Math Operator Expressions
	//////////////////////////////////////////////

	static Expr
		ParseAdditionExpr(Token& fp_CurrentToken, vector<Token>& fp_Tokens)
	{

	}

	static Expr
		ParseSubtractionExpr()
	{

	}

	static Expr
		ParseMultiplicationExpr()
	{

	}

	static Expr
		ParseDivisionExpr()
	{

	}

	static bool
		ParseUserIdentifier(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, Expr* fp_UserIdentifier)
	{
		fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for any accessor symbols
		fp_ProgramCounter++;

		switch(fp_CurrentToken.m_Type)
		{
		case TokenType::Dot: //used for class calling, eg. "myClassInstance.myMethod(func args);" or "myClassInstance.myField = 69;"
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens);
			fp_ProgramCounter++;

			//if we arent accessing a method or field after the ".", then idc wtf u typed, that shit is getting thrown out dawg
			if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Something bad happened while calling a class method! Make sure you're calling the proper method name", "Parser", "warn", "yellow");
				return false;
			}

			//TODO: figure out how to differentiate between nested classes and methods
			while (fp_ProgramTokens.size() > 0 && fp_CurrentToken.m_Type != TokenType::SemiDot)
			{

			}


		}
			break;
		case TokenType::Equals:
		{

		}
			break;
		case TokenType::StrictlyEquals:
		{

		}
		case TokenType::OpenParen:
		{

		}
			break;
		case TokenType::As: //used for casting class instances, eg. myFunc( myClass as otherClass); (we call this func in parseFunc)
		{

		}
			break;
		case TokenType::AdditionOperator: //used for: "myFunc( otherFunc() + 3, otherFunc() + anotherFunc())" or "func myFunc(
		{

		}
			break;
		case TokenType::NegativeOperator:
		{

		}
		break;
		case TokenType::MultiplicationOperator:
		{

		}
		break;
		case TokenType::DivisionOperator:
		{

		}
		break;
		case TokenType::PlusEqualsOperator:
		{

		}
		break;
		case TokenType::MinusEqualsOperator:
		{

		}
		break;
		case TokenType::MultEqualsOperator:
		{

		}
		break;
		case TokenType::DivEqualsOperator:
		{

		}
		break;

		default:
			break;
		}

		return true;
	}

	//////////////////////////////////////////////
	// Conditional Logic Blocks
	//////////////////////////////////////////////

	static bool
		ParseIfBlock(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, IfDeclaration& fp_VariableDefinition)
	{
		vector<Expr> f_Condition;
		vector<Expr> f_Body;
		unsigned int f_BracketDepthTracker = 0; //tracks the bracket level, we add as we find open parens, and subtract as we find closed parens

		fp_CurrentToken = ShiftForward(fp_ProgramTokens);
		fp_ProgramCounter++;

		if (fp_CurrentToken.m_Type != TokenType::OpenParen)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your if statement brother! Try looking at your brackets on the if-statement", "Parser", "warn", "yellow");
			return false;
		}
		else //if it"s an open paren then we"re clear to move forward and read the condition inside
		{
			f_BracketDepthTracker++; //add one to the depth to start off with, this should reset to 0 for proper syntax
		}

		//parse through the condition "( cond )"
		while (fp_ProgramTokens.size() > 0 && f_BracketDepthTracker > 0)
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic
			fp_ProgramCounter++;

			switch(fp_CurrentToken.m_Type)
			{
			case TokenType::CloseParen:
			{
				f_BracketDepthTracker--;
				continue;
			}
			break;
			case TokenType::OpenParen:
			{
				f_BracketDepthTracker++;
				continue;
			}
			break;
			case TokenType::FloatNumber: //meant to fall through
			case TokenType::IntNumber:
			{
				Expr s_NumberExpr;

				if (!ParseNumber(fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_NumberExpr))
				{
					return false;
				}

				f_Condition.push_back(s_NumberExpr);
			}
			break;
			case TokenType::StringLiteral:
				break;
			case TokenType::UserIdentifier: //used for
			{
				Expr s_UserDefinedAction; //used for var reassignment, class method access

				if (!ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_UserDefinedAction))
				{
					return false;
				}

				f_Condition.push_back(s_UserDefinedAction);
			}
			break;
			case TokenType::And:
				break;
			case TokenType::Or:
				break;
			case TokenType::Not:
				break;
			default:
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser", "warn", "yellow");
				return false;
			break;
			}
		}
	}

	static WhileLoopDeclaration
		ParseWhileBlock()
	{

	}

	static ForLoopDeclaration
		ParseForBlock()
	{

	}

	static bool
		ParseFuncDeclaration(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, FuncDeclaration* fp_FuncBlock)
	{
		unsigned int f_BracketDepthTracker = 0; //tracks the bracket level, we add as we find open parens, and subtract as we find closed parens

		fp_CurrentToken = ShiftForward(fp_ProgramTokens); //assuming this is being called when fp_CurrentToken == TokenType::Func
		fp_ProgramCounter++;

		if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your function brother! Try taking a look at your function name definition", "Parser", "warn", "yellow");
			return false;
		}
		else //if it"s a proper name defintion then we can now proceed forwards
		{
			fp_FuncBlock->m_FuncName = fp_CurrentToken; //record the function name
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;
		}

		if (fp_CurrentToken.m_Type != TokenType::OpenParen)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your function brother! Try taking a look at how you've placed your parenthesis", "Parser", "warn", "yellow");
			return false;
		}
		else //if it"s an open paren then we"re clear to move forward and read the condition inside
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;
		}

		bool f_StillInFunctionArgDefinition = true;

		while(f_StillInFunctionArgDefinition)
		{
			if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //handles variables, function, and class instance names being passed as a single argument, could use this for some semi-dynamic typing xdxd
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized symbol following an open-bracket while declaring your function arguments brother! Try taking a look at your function argument(s) defintion", "Parser", "warn", "yellow");
				return false;
			}

			fp_FuncBlock->m_FuncArgs.push_back(fp_CurrentToken);
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;


			if (fp_CurrentToken.m_Type != TokenType::TypeArrow)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized symbol following a name definition while declaring your function arguments brother! Try taking a look at your type-arrows", "Parser", "warn", "yellow");
				return false;
			}

			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;

			if (
				fp_CurrentToken.m_Type != TokenType::Int &&
				fp_CurrentToken.m_Type != TokenType::Float &&
				fp_CurrentToken.m_Type != TokenType::Bool &&
				fp_CurrentToken.m_Type != TokenType::String &&
				fp_CurrentToken.m_Type != TokenType::UserIdentifier
				)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized type found while declaring your function brother! Are you sure you've entered a valid type in your function argument definition?", "Parser", "warn", "yellow");
				return false;
			}

			fp_FuncBlock->m_FuncArgTypes.push_back(fp_CurrentToken);
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;

			if(fp_CurrentToken.m_Type != TokenType::Comma)
			{
				if (fp_CurrentToken.m_Type == TokenType::CloseParen)
				{
					f_StillInFunctionArgDefinition = false;
				}
				else
				{
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("Something bad happened while calling a class method! Make sure you're calling the proper method name", "Parser", "warn", "yellow");
					return false;
				}
			} //end of scope
			else if (fp_CurrentToken.m_Type == TokenType::ENDF)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Something bad happened while calling a class method! Make sure you're calling the proper method name", "Parser", "warn", "yellow");
				return false;
			}
		}
		return true;
	}

	static bool
		ParseFunctionCall(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, FunctionCallExpr* fp_FuncCall)
	{
		unsigned int f_BracketDepthTracker = 0; //tracks the bracket level, we add as we find open parens, and subtract as we find closed parens

		fp_CurrentToken = ShiftForward(fp_ProgramTokens);
		fp_ProgramCounter++;

		if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your function brother! Try taking a look at your function name definition", "Parser", "warn", "yellow");
			return false;
		}
		else //if it"s a proper name defintion then we can now proceed forwards
		{
			fp_FuncCall->m_FuncName = fp_CurrentToken; //record the function name
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;
		}

		if (fp_CurrentToken.m_Type != TokenType::OpenParen)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your function brother! Try taking a look at how you've placed your parenthesis", "Parser", "warn", "yellow");
			return false;
		}
		else //if it"s an open paren then we"re clear to move forward and read the condition inside
		{
			f_BracketDepthTracker++; //add one to the depth to start off with, this should reset to 0 for proper syntax
		}

		bool f_ShouldShiftFuncArgs = false;

		//FUNCTION ARGUMENTS
		while (fp_ProgramTokens.size() > 0 && f_BracketDepthTracker > 0)
		{
			if (f_ShouldShiftFuncArgs) //used for accounting for over stepped tokens in the while loop logic
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
				fp_ProgramCounter++;
			}
			else
			{
				f_ShouldShiftFuncArgs = true;
			}

			switch (fp_CurrentToken.m_Type)
			{
			case TokenType::CloseParen:
				f_BracketDepthTracker--;
				continue; // I THINK THIS WORKS THE WAY I WANT IT TO
			case TokenType::OpenParen:
				f_BracketDepthTracker++;
				continue;
			case TokenType::UserIdentifier: //handles variables, function, and class instance names being passed as a single argument
			{
				Expr s_UserDefinedAction; //used for var reassignment, class method access

				if (!ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_UserDefinedAction))
				{
					return false;
				}

				f_ShouldShiftFuncArgs = false; //shouldn"t shift since we over stepped a token at the beginning of this case statement
				continue; //we continue since we"ve over stepped a character

			} //end of scope
			break;
			case TokenType::Comma:
			{

			} //end of scope
			break;
			case TokenType::FloatNumber:
			{

			} //end of scope
			break;
			case TokenType::StringLiteral:
			{

			} //end of scope
			break;
			case TokenType::True:
			{

			} //end of scope
			break;
			case TokenType::False:
			{

			} //end of scope 
			break;
			default:
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Something bad happened while calling a class method! Make sure you're calling the proper method name", "Parser", "warn", "yellow");
				return false;
			}
			break;
			}
		}
		return true;
	}

	static bool
		ParseClassDeclaration(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, ClassDeclaration& fp_ClassBlock, bool fp_IsSingle)
	{
		fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an user defined identifier eg. "class MyClass"
		fp_ProgramCounter++;

		if (fp_CurrentToken.m_Type == TokenType::UserIdentifier)//if it"s a proper name defintion then we can now proceed forwards
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;

			if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser", "warn", "yellow");
				return false;
			}

			while (fp_ProgramTokens.size() > 0 && fp_CurrentToken.m_Type != TokenType::CloseBracket)
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward since the current character is "{"
				fp_ProgramCounter++;
			}
		}
		else
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser", "warn", "yellow");
			return false;
		}

		return true;
	}

	static bool
		ParseStructDeclaration(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, StructDeclaration& fp_StructBlock)
	{
		fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an user defined identifier eg. "class MyClass"
		fp_ProgramCounter++;

		if (fp_CurrentToken.m_Type == TokenType::UserIdentifier)//if it"s a proper name defintion then we can now proceed forwards
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
			fp_ProgramCounter++;

			if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser", "warn", "yellow");
				return false;
			}

			while (fp_ProgramTokens.size() > 0 && fp_CurrentToken.m_Type != TokenType::CloseBracket)
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward since the current character is "{"
				fp_ProgramCounter++;
			}
		}
		else
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser", "warn", "yellow");
			return false;
		}

		return true;
	}

	static bool
		ParseScopeDeclaration(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, ScopeDeclaration& fp_ScopeBlock)
	{

	}

	static ListDeclaration
		ParseListDeclaration()
	{

	}

	static DictionaryDeclaration
		ParseDictionaryDeclaration()
	{

	}

	static bool //returns true if it worked, false if it failed
		ParseLetDeclaration(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, Program& fp_Program, LetDeclaration& fp_VariableDefinition)
	{
		fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for any accessor symbols
		fp_ProgramCounter++;

		if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring a variable brother! Try taking a look at the variable name definition", "Parser", "warn", "yellow");
			return false;
		}
		else
		{
			fp_VariableDefinition.m_VariableName = fp_CurrentToken;
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
			fp_ProgramCounter++;
		}

		if (fp_CurrentToken.m_Type != TokenType::TypeArrow)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Invalid symbol found after name-definition while declaring a variable brother! Try taking a look at the variable type-arrow definition", "Parser", "warn", "yellow");
			return false;
		}
		else
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for type and member access level
			fp_ProgramCounter++;
		}

		if (fp_CurrentToken.m_Type == TokenType::Public || fp_CurrentToken.m_Type == TokenType::Protected)
		{

		}

		if (
			fp_CurrentToken.m_Type != TokenType::UserIdentifier && //used for user defined types, if this isn"t a type we"ll catch it as a runtime error
			fp_CurrentToken.m_Type != TokenType::Int && //actually refering to the type itself and not the literal value
			fp_CurrentToken.m_Type != TokenType::Float &&
			fp_CurrentToken.m_Type != TokenType::String &&
			fp_CurrentToken.m_Type != TokenType::Bool
			)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Something bad happened while declaring a variable brother! Try taking a look at the variable type definition", "Parser", "warn", "yellow");
			return false;
		}
		else
		{
			fp_VariableDefinition.m_VariableType = fp_CurrentToken;
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
			fp_ProgramCounter++;
		}

		if (fp_CurrentToken.m_Type != TokenType::Equals)
		{
			//THROW ERROR
			LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
			LogAndPrint("Unexpected symbol found after type-indicator while declaring a variable brother! Try taking a look at the variable type definition", "Parser", "warn", "yellow");
			return false;
		}
		else
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
			fp_ProgramCounter++;
		}

		while (fp_ProgramTokens.size() > 0 && fp_CurrentToken.m_Type != TokenType::SemiDot) //parse equality expression
		{
			switch (fp_CurrentToken.m_Type)
			{
			case TokenType::IntNumber:
				break;
			case TokenType::FloatNumber:
				break;
			case TokenType::StringLiteral:
				break;
			case TokenType::OpenBracket:
				break;
			case TokenType::New:
			{
				if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
				{

				}
				else
				{

				}
			}
				break;
			case TokenType::UserIdentifier: //used for: var x -> Car: new Car();, var x -> Car = new Car();
			{
				if (!FindStringInVector(fp_Program.m_ListClassNames, fp_CurrentToken.m_Value))
				{
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("Undefined class type found while declaring a variable brother! Are you defining this variable before your class definition?", "Parser", "warn", "yellow");
					return false;
				}
			}
				break;
			default:
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Invalid value found while declaring a variable brother! Try taking a look at the variable value definition", "Parser", "warn", "yellow");
				return false;
				break;
			}
		}

		return true;
	}

	//////////////////////////////////////////////
	// Parse Standard Library Functions
	//////////////////////////////////////////////
	
	static bool
		ParsePrintFunction(Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, PrintFunction& fp_PrintFunctionCall)
	{

		return true;
	}

	//////////////////////////////////////////////
	// Main Processing Function
	//////////////////////////////////////////////

	static Program 
		ConstructAST(string fp_SourceCode)
	{
		vector<Token> f_ProgramTokens = Tokenize(fp_SourceCode);
		vector<StatementNode> f_ProgramStatements;

		Program f_Program;

		size_t f_ProgramCounter = 0;

		Token f_CurrentToken;

		bool f_IsSingle = false; //used for tracking whether the class defintion we're in is a singleton or not

		while (!IsENDF(f_CurrentToken)) //this loop operates on every statement that lives on scope-depth = 0, everything else is branched like a tree from the 0th level
		{
			f_CurrentToken = ShiftForward(f_ProgramTokens);
			f_ProgramCounter++;

			switch(f_CurrentToken.m_Type)
			{
				case TokenType::OpenBracket: //we want to handle scope declarations first since any keywords will overshadow it in the logical flow
				{
					ScopeDeclaration s_ScopeBlock;

					if (!ParseScopeDeclaration(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, s_ScopeBlock))
					{
						f_ProgramTokens.clear(); //dump source code so that lexical analysis ends immediately
						continue;
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_ScopeBlock);
					}
				}
					break;
				case TokenType::If:
				{
					IfDeclaration s_IfStatement;

					if (!ParseIfBlock(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, s_IfStatement))
					{
						f_ProgramTokens.clear(); //dump source code so that lexical analysis ends immediately
						continue;
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_IfStatement);
					}
				}
					break;
				case TokenType::Let:
				{
					LetDeclaration s_VariableDefinition;

					if (!ParseLetDeclaration(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, f_Program, s_VariableDefinition))
					{
						f_ProgramTokens.clear(); //dump source code so that lexical analysis ends immediately
						continue;
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_VariableDefinition);
					}
				}
					break;
				case TokenType::UserIdentifier: //deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();"
				{
					Expr s_UserDefinedAction; //used for var reassignment, class method access

					if (!ParseUserIdentifier(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, &s_UserDefinedAction))
					{
						f_ProgramTokens.clear(); //dump source code so that lexical analysis ends immediately
						continue; //already dumped src code in the parse function
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_UserDefinedAction);
					}
				}
					break;
				case TokenType::Func:
				{
					FuncDeclaration s_FunctionDefintion; //used for func args, var reassignment, class method access

					if (!ParseFuncDeclaration(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, &s_FunctionDefintion))
					{
						f_ProgramTokens.clear(); //dump source code so that lexical analysis ends immediately
						continue;
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_FunctionDefintion);
					}

				}
					break;
				case TokenType::For:
				{

				}
					break;
				case TokenType::While:
				{
					
				}
					break;
				case TokenType::Print:
				{

				}
					break;
				case TokenType::Single:
					//do stuff and fall into class case logic
					f_IsSingle = true;
					//WE PURPOSELY DONT HAVE A BREAK HERE ITS MEANT TO FALL INTO CLASS SINCE ITS STRICTLY A CLASS MODIFIER
				case TokenType::Class: //Class Definition, not accessor; we do that with the UserIdentifier token
				{
					ClassDeclaration s_ClassBlock; //used for func args, var reassignment, class method access

					if (!ParseClassDeclaration(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, s_ClassBlock, f_IsSingle))
					{
						continue;
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_ClassBlock);
						f_IsSingle = false; //reset so that the next class definition doesn't get falsely labelled as a singleton
					}
				}
					break;
				case TokenType::Struct: //define a struct
				{
					StructDeclaration s_StructBlock; //used for func args, var reassignment, class method access

					if (!ParseStructDeclaration(f_CurrentToken, f_ProgramTokens, f_ProgramCounter, s_StructBlock))
					{
						continue;
					}
					else
					{
						f_Program.m_ProgramBody.push_back(s_StructBlock);
					}
				}
					break;
				default:
					LogAndPrint("Compiler Error: Improper grammar found in source code at line " + to_string(f_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					f_ProgramTokens.clear(); //dump all tokens, so that the compiler will stop processing the source code
					break;
			}

		}


	}
}