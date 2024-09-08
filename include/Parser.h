/*****************************************************************
 *                                        BongoJam Script v0.3                                        
 *                           Created by Ranyodh Mandur - © 2024                            
 *                                                                                                                  
 *                         Licensed under the MIT License (MIT).                           
 *                  For more details, see the LICENSE file or visit:                     
 *                        https://opensource.org/licenses/MIT                               
 *                                                                                                                  
 * BongoJam is an open-source scripting language compiler and interpreter 
 *              primarily intended for embedding within game engines.               
 *****************************************************************/

#pragma once

#include <cstring> // for memcpy

#include "AST.h"
#include "Lexer.h"

namespace BongoJam {
	class Parser
	{
		//////////////////////////////////////////////
		// Utility Functions
		//////////////////////////////////////////////

		 bool
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

		 Token
			ShiftForward(vector<Token>&fp_TokenVector)
		{
			if (fp_TokenVector.empty())
			{
				return Token("\0", TokenType::ENDF, -1); //return escape char when source code is done being read
			}

			Token f_FirstElement = fp_TokenVector.front();
			fp_TokenVector.erase(fp_TokenVector.begin());
			return f_FirstElement;
		}

		 bool
			FindStringInVector(const vector<string>&fp_Vector, const string & fp_DesiredString)
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

		 bool
			ParseRegularExpr() //used for parsing any expr, and types it accordingly
		{

		}

		//this method will find the smallest possible expression formed by the tokens following the number token, and return it
		 bool
			ParseNumber(Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, Expr * fp_NumberExpr)
		{


			return true;
		}

		//////////////////////////////////////////////
		// Math Operator Expressions
		//////////////////////////////////////////////

		 Expr
			ParseAdditionExpr(Token & fp_CurrentToken, vector<Token>&fp_Tokens)
		{

		}

		 Expr
			ParseSubtractionExpr()
		{

		}

		 Expr
			ParseMultiplicationExpr()
		{

		}

		 Expr
			ParseDivisionExpr()
		{

		}

		//this function is used to deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();"		    handled by ParseNumber()
		//we also deal with expressions formed within method or function calls, so this method will return an expression ending with ';' or ',' eg. myFunc(1, 3 + otherFunc(), otherFunc() * 2);
		//																																					called					      called
		 bool
			ParseUserIdentifier(uint32_t & fp_CurrentScopeDepth, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, Expr * fp_UserIdentifier)
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for any accessor symbols
			fp_ProgramCounter++;

			switch (fp_CurrentToken.m_Type)
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

		bool
			ParseIfBlock(uint32_t & fp_CurrentScopeDepth, bool fp_IsInLoop, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t &fp_ProgramCounter, unique_ptr<IfDeclaration>* fp_IfStatement, unique_ptr<IfDeclaration>* fp_ElseIfStatement = nullptr)
		{
			unsigned int f_BracketDepthTracker = 0; //tracks the bracket level, we add as we find open parens, and subtract as we find closed parens
			unique_ptr<IfDeclaration>* f_CurrentStatement = nullptr; //either an if or else-if statement

			if (fp_ElseIfStatement)
			{
				f_CurrentStatement = fp_ElseIfStatement;
			}
			else
			{
				f_CurrentStatement = fp_IfStatement;
			}

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

			//if fp_IsElse is true, then this will skip over the condition section uwu

			//parse through the condition "( cond )"
			while (fp_ProgramTokens.size() > 0 && f_BracketDepthTracker > 0)
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic
				fp_ProgramCounter++;

				switch (fp_CurrentToken.m_Type)
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

					(*f_CurrentStatement)->m_Condition.push_back(s_NumberExpr);
				}
				break;
				case TokenType::StringLiteral:
					break;
				case TokenType::UserIdentifier: //used for
				{
					Expr s_UserDefinedAction; //used for var reassignment, class method access

					if (!ParseUserIdentifier(fp_CurrentScopeDepth, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_UserDefinedAction))
					{
						return false;
					}

					(*f_CurrentStatement)->m_Condition.push_back(s_UserDefinedAction);
				}
				break;
				case TokenType::And:
					break;
				case TokenType::Or:
					break;
				case TokenType::Not:
					break;
					break;
				default:
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser", "warn", "yellow");
					return false;
					break;
				}
			}

			assert(fp_CurrentToken.m_Type == TokenType::CloseParen);

			//fp_CurrentToken = ')' right now
			//assuming that when the bracket count goes to 0, we are outside of the if statement condition, we now check for the first curly-brace
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now to look for an open bracket
			fp_ProgramCounter++;

			if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser", "warn", "yellow");
				return false;
			}

			//parse the if-statement code body
			while (fp_ProgramTokens.size() > 0 && f_BracketDepthTracker > 0)
			{

				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic
				fp_ProgramCounter++;

				switch (fp_CurrentToken.m_Type)
				{
				case TokenType::Return:
				{
				}
				break;
				case TokenType::Break:
				{
					if (!fp_IsInLoop)
					{
						//THROW ERROR
						LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
						LogAndPrint("You are not allowed to use a break statement outside of a loop brother!", "Parser", "warn", "yellow");
						return false;
					}
					(*f_CurrentStatement)->m_CodeBody.push_back(make_unique<BreakSubStatement>());
				}
				break;
				case TokenType::If:
				{
					unique_ptr<IfDeclaration> s_NestedIfStatement = make_unique<IfDeclaration>();

					if (!ParseIfBlock(fp_CurrentScopeDepth, fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_NestedIfStatement))
					{
						fp_ProgramTokens.clear();
						return false;
					}
					else
					{
						(*f_CurrentStatement)->m_CodeBody.push_back(move(s_NestedIfStatement));
						continue;
					}
				}
				break;
				default:
					break;
				}
			}

			if (fp_CurrentToken.m_Type != TokenType::CloseBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser", "warn", "yellow");
				return false;
			}

			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic
			fp_ProgramCounter++;

			//we do a recursive call here since everything for if also applies for else-if, and it will just repeatedly keep hitting this statement until we hit an else or nothing
			// 
			//this top call is for else-if chains longer than 1, this logic flow ensures that the second if, starts an else-if chain, and the first if maintains the if statement as the primary list holder
			//opposed to each else-if statement chaining to each other like a tree, making iteration a nightmare, needing a traversal across pointer de-referencing across n-layers of the tree oof
			if (fp_CurrentToken.m_Type == TokenType::Elif && fp_ElseIfStatement)
			{
				unique_ptr<IfDeclaration> f_FallThroughCondition = make_unique<IfDeclaration>();

				if (!ParseIfBlock(fp_CurrentScopeDepth, fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, (fp_IfStatement), &(f_FallThroughCondition)))
				{
					fp_ProgramTokens.clear();
					return false;
				}
				else
				{
					(*fp_IfStatement)->m_ElseIfStatements.push_back(move(f_FallThroughCondition));
				}
			}
			else if (fp_CurrentToken.m_Type == TokenType::Elif)
			{
				unique_ptr<IfDeclaration> f_FallThroughCondition = make_unique<IfDeclaration>();

				if (!ParseIfBlock(fp_CurrentScopeDepth, fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, (f_CurrentStatement), &(f_FallThroughCondition)))
				{
					fp_ProgramTokens.clear();
					return false;
				}
				else
				{
					(*f_CurrentStatement)->m_ElseIfStatements.push_back(move(f_FallThroughCondition));
				}
			}
			else if (fp_CurrentToken.m_Type == TokenType::Else)
			{
				unique_ptr<ElseDeclaration> f_ElseStatement = make_unique<ElseDeclaration>();

				if (!ParseElseBlock(fp_CurrentScopeDepth, fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, (f_CurrentStatement), &(f_ElseStatement)))
				{
					fp_ProgramTokens.clear();
					return false;
				}
				else// THIS IS NOT COCHER, THIS WILL LEAVE A DANGLING PTR REF AFTER THE SCOPE HAS EXITED
				{
					(*f_CurrentStatement)->m_ElseStatement = move((f_ElseStatement));
				}
			}

			return true;
		}

		bool
			ParseElseBlock(uint32_t & fp_CurrentScopeDepth, bool fp_IsInLoop, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, unique_ptr<IfDeclaration>* fp_IfStatement, unique_ptr<ElseDeclaration>* fp_ElseBlock)
		{
			unsigned int f_BracketDepthTracker = 0; //tracks the bracket level, we add as we find open parens, and subtract as we find closed parens

			fp_CurrentToken = ShiftForward(fp_ProgramTokens);
			fp_ProgramCounter++;

			if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized symbol found in your else-block brother! Try taking a look at your '{ }' code-body wrappers", "Parser", "warn", "yellow");
				return false;
			}

			//parse the else code body
			while (fp_ProgramTokens.size() > 0 && f_BracketDepthTracker > 0)
			{

				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic
				fp_ProgramCounter++;

				switch (fp_CurrentToken.m_Type)
				{
				case TokenType::Return:
				{
				}
				break;
				case TokenType::Break:
				{
					if (!fp_IsInLoop)
					{
						//THROW ERROR
						LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
						LogAndPrint("You are not allowed to use a break statement outside of a loop brother!", "Parser", "warn", "yellow");
						return false;
					}
					//(*fp_IfStatement)->m_CodeBody.push_back(make_unique<BreakSubStatement>());
				}
				break;
				case TokenType::If:
				{
					unique_ptr<IfDeclaration> s_NestedIfStatement = make_unique<IfDeclaration>();

					if (!ParseIfBlock(fp_CurrentScopeDepth, fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &(s_NestedIfStatement)))
					{
						fp_ProgramTokens.clear();
						return false;
					}
					else
					{
						(*fp_IfStatement)->m_CodeBody.push_back(move(s_NestedIfStatement));
						continue;
					}
				}
				break;
				default:
					break;
				}
			}

			if (fp_CurrentToken.m_Type != TokenType::CloseBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser", "warn", "yellow");
				return false;
			}

			return true;
		}

		 WhileLoopDeclaration
			ParseWhileBlock()
		{

		}

		 ForLoopDeclaration
			ParseForBlock()
		{

		}

		bool
			ParseFuncDeclaration(uint32_t& fp_CurrentScopeDepth, Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, Program* fp_Program, size_t& fp_ProgramCounter, unique_ptr<FuncDeclaration>* fp_FuncBlock)
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
				(*fp_FuncBlock)->m_FuncName = fp_CurrentToken; //record the function name
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

			while (f_StillInFunctionArgDefinition)
			{
				if (fp_CurrentToken.m_Type == TokenType::CloseParen) //handles variables, function, and class instance names being passed as a single argument, could use this for some semi-dynamic typing xdxd
				{
					break;
				}

				if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //handles variables, function, and class instance names being passed as a single argument, could use this for some semi-dynamic typing xdxd
				{
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("Unrecognized symbol following an open parenthesis while declaring: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s arguments brother! Try taking a look at your function argument(s) defintion", "Parser", "warn", "yellow");
					return false;
				}

				(*fp_FuncBlock)->m_FuncArgs.push_back(fp_CurrentToken);
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
				fp_ProgramCounter++;


				if (fp_CurrentToken.m_Type != TokenType::TypeArrow)
				{
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("Unrecognized symbol following a name definition while declaring: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s arguments brother! Try taking a look at your type-arrows", "Parser", "warn", "yellow");
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
					LogAndPrint("Unrecognized type found while declaring: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s brother! Are you sure you've entered a valid type in " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s argument definition?", "Parser", "warn", "yellow");
					return false;
				}

				(*fp_FuncBlock)->m_FuncArgTypes.push_back(fp_CurrentToken); //type indices correspond to the func arg list
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
				fp_ProgramCounter++;
				//TODO: ADD A CASE FOR EQUALS AND FIGURE OUT A WAY TO PARSE DEFAULT'D PARAMETERS
				if (fp_CurrentToken.m_Type != TokenType::Comma)
				{
					if (fp_CurrentToken.m_Type == TokenType::CloseParen)
					{
						f_StillInFunctionArgDefinition = false;
						break;
					}
					else
					{
						//THROW ERROR
						LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
						LogAndPrint("Unexpected symbol found in: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s argument defintion! \n Try taking a look at your comma separation between function parameters", "Parser", "warn", "yellow");
						return false;
					}
				} //end of scope
				else if (fp_CurrentToken.m_Type == TokenType::ENDF)
				{
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("End of file found instead of a closed parenthesis inside: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s definition! \n Make sure you end your function parameter definition with a ')'", "Parser", "warn", "yellow");
					return false;
				}
			}

			//if function definition was valid, then f_CurrentToken = '(' right now
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for a type arrow definition
			fp_ProgramCounter++;

			if (fp_CurrentToken.m_Type != TokenType::TypeArrow)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unexpected symbol found in: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s return type defintion! \n Try taking a look at your type-arrow definition", "Parser", "warn", "yellow");
				return false;
			}
			else
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for a return type definition
				fp_ProgramCounter++;
			}

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
				LogAndPrint("Invalid type found in: " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s return type defintion! \n Try taking a look at your type-arrow definition", "Parser", "warn", "yellow");
				return false;
			}
			else
			{
				(*fp_FuncBlock)->m_FuncReturnType = fp_CurrentToken; //assign return type to the FuncDeclaration struct
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open bracket to indicate function body code
				fp_ProgramCounter++;
			}

			if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unrecognized symbol following " + (*fp_FuncBlock)->m_FuncName.m_Value + "'s function parameter definition! Try taking a look at your function body definition", "Parser", "warn", "yellow");
				return false;
			}
			else
			{
				fp_CurrentScopeDepth++; //now that we're inside a bracket, we can increment the scope depth confidently
				//we don't shift here because we want the while loop to always shift at the top, so we just leave it for the first iteration uwu
			}

			//parse function body definition
			while (fp_CurrentToken.m_Type != TokenType::CloseBracket && fp_ProgramTokens.size() > 0)
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to start parsing the function body
				fp_ProgramCounter++;

				switch (fp_CurrentToken.m_Type)
				{
				case TokenType::Print:
				{
					unique_ptr<PrintFunction> s_PrintFunctionCall = make_unique<PrintFunction>();

					if (!ParsePrintFunction(fp_CurrentScopeDepth, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_PrintFunctionCall))
					{
						fp_ProgramTokens.clear(); //dump source code so that parsing ends immediately
						return false;
					}
					else
					{
						(*fp_FuncBlock)->m_CodeBody.push_back(move(s_PrintFunctionCall));
						continue; //current token should be ';' should be safe to shift
					}
					//after parsing the print function successfully we should be pointing to an already processed token, so we can safely iterate and ShiftForward() at the top of the loop
				}
				break;
				case TokenType::UserIdentifier: //deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();" specifically
				{
					Expr s_UserDefinedAction; //used for var reassignment, class method access

					if (!ParseUserIdentifier(fp_CurrentScopeDepth, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_UserDefinedAction))
					{
						fp_ProgramTokens.clear(); //dump source code so that parsing ends immediately
						return false;
					}
					else
					{
						//fp_FuncBlock->m_CodeBody.push_back(s_UserDefinedAction);
						continue; //current token should be ';' should be safe to shift
					}
				}
				break;
				case TokenType::OpenBracket: //we want to handle scope declarations first since any keywords will overshadow it in the logical flow
				{
					ScopeDeclaration s_ScopeBlock;

					if (!ParseScopeDeclaration(fp_CurrentScopeDepth, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, s_ScopeBlock))
					{
						fp_ProgramTokens.clear(); //dump source code so that parsing ends immediately
						return false;
					}
					else
					{
						//fp_FuncBlock->m_CodeBody.push_back(s_ScopeBlock);
						continue; //current token should be '}' should be safe to shift
					}
				}
				break;
				case TokenType::Let:
				{
					LetDeclaration s_VariableDefinition;

					if (!ParseLetDeclaration(fp_CurrentScopeDepth, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, fp_Program, s_VariableDefinition))
					{
						fp_ProgramTokens.clear(); //dump source code so that parsing ends immediately
						return false;
					}
					else
					{
						//fp_FuncBlock->m_CodeBody.push_back(s_VariableDefinition);
						continue; //current token should be ';' should be safe to shift
					}
				}
				break;
				case TokenType::If:
				{
					unique_ptr<IfDeclaration> s_IfStatement = make_unique<IfDeclaration>();

					if (!ParseIfBlock(fp_CurrentScopeDepth, false, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &(s_IfStatement)))
					{
						fp_ProgramTokens.clear(); //dump source code so that parsing ends immediately
						continue;
					}
					else
					{
						(*fp_FuncBlock)->m_CodeBody.push_back(move(s_IfStatement));
						continue;
					}
				}
				break;
				case TokenType::While:
					break;
				case TokenType::For:
					break;
				case TokenType::Try:
					break;
				default:
					break;
				}
			}

			//since we successfully parsed the function body, due to our while-loop condition, f_CurrentToken = '}',
			//we don't need to ShiftForward() here because when this function exits, it leaves that responsibility up to the while loop that called it
			//so fp_CurrentToken = '}' aka close bracket

			return true;
		}

		 bool
			ParseFunctionCall(uint32_t & fp_CurrentScopeDepth, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, FunctionCallExpr * fp_FuncCall)
		{
			unsigned int f_BracketDepthTracker = 0; //tracks the bracket level, we add as we find open parens, and subtract as we find closed parens

			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //assuming we're being called from 
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

					if (!ParseUserIdentifier(fp_CurrentScopeDepth, fp_CurrentToken, fp_ProgramTokens, fp_ProgramCounter, &s_UserDefinedAction))
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

		 bool
			ParseClassDeclaration(uint32_t & fp_CurrentScopeDepth, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, ClassDeclaration & fp_ClassBlock, bool fp_IsSingle)
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

		 bool
			ParseStructDeclaration(uint32_t& fp_CurrentScopeDepth, Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, StructDeclaration& fp_StructBlock)
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

		 bool
			ParseScopeDeclaration(uint32_t & fp_CurrentScopeDepth, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, ScopeDeclaration & fp_ScopeBlock)
		{
			 return true;
		}

		 ListDeclaration
			ParseListDeclaration()
		{

		}

		 DictionaryDeclaration
			ParseDictionaryDeclaration()
		{

		}

		 bool //returns true if it worked, false if it failed
			ParseLetDeclaration(uint32_t & fp_CurrentScopeDepth, Token & fp_CurrentToken, vector<Token>&fp_ProgramTokens, size_t & fp_ProgramCounter, Program* fp_Program, LetDeclaration & fp_VariableDefinition)
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
					if (!FindStringInVector(fp_Program->m_ListClassNames, fp_CurrentToken.m_Value))
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

		 bool
			 ParsePrintFunction(uint32_t& fp_CurrentScopeDepth, Token& fp_CurrentToken, vector<Token>& fp_ProgramTokens, size_t& fp_ProgramCounter, unique_ptr<PrintFunction>* fp_PrintFunctionCall)
		{
			fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for '('
			fp_ProgramCounter++;

			if (fp_CurrentToken.m_Type != TokenType::OpenParen)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unexpected symbol found when '(' was expected during a print() call!", "Parser", "warn", "yellow");
				return false;
			}
			else
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a string token
				fp_ProgramCounter++;
			}

			if (fp_CurrentToken.m_Type != TokenType::StringLiteral)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Tried to pass a non-text data type when text was expected! Try taking a look at your print() call argument", "Parser", "warn", "yellow");
				return false;
			}
			else
			{
				(*fp_PrintFunctionCall)->m_FuncArgs.push_back(make_unique<StringLiteral>(fp_CurrentToken));
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
				fp_ProgramCounter++;
			}

			//check for an optional colour parameter
			if (fp_CurrentToken.m_Type == TokenType::Comma)
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for another string literal that indicates the colour
				fp_ProgramCounter++;

				if (fp_CurrentToken.m_Type != TokenType::StringLiteral)
				{
					//THROW ERROR
					LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					LogAndPrint("Tried to pass a non-text data type when text was expected! Try taking a look at your print() call colour argument", "Parser", "warn", "yellow");
					return false;
				}
				else
				{
					(*fp_PrintFunctionCall)->m_FuncArgs.push_back(make_unique<StringLiteral>(fp_CurrentToken)); //colour arg
					fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
					fp_ProgramCounter++;
				}
			}

			//we overstep a token if the above arguments for print are valid, so no need to shift again

			if (fp_CurrentToken.m_Type != TokenType::CloseParen)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unexpected symbol found when ')' was expected during a print() call!", "Parser", "warn", "yellow");
				return false;
			}
			else
			{
				fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a string token
				fp_ProgramCounter++;
			}

			if (fp_CurrentToken.m_Type != TokenType::SemiDot)
			{
				//THROW ERROR
				LogAndPrint("Error at Line Number: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
				LogAndPrint("Unexpected symbol found when ';' was expected after a call to print()", "Parser", "warn", "yellow");
				return false;
			}
			//we let the main while loop ShiftForward() off of the ';'

			return true;
		}

		//ValidateAST's main job is to check for things like scope errors, where a variable is being referenced outside its scope of definition
		 bool
			ValidateAST(vector<StatementNode>& fp_ProgramStatements)
		{
			 return true;
		}
	public:
		//////////////////////////////////////////////
		// Main Processing Function
		//////////////////////////////////////////////

		Program*
			ConstructAST(vector<Token> f_ProgramTokens)
		{
			vector<StatementNode> f_ProgramStatements;

			map<string, uint32_t> f_VariableToScopeMap;

			Program* f_Program = new Program();

			size_t f_ProgramCounter = 0;
			uint32_t f_CurrentScopeDepth = 0; //0 indicates global scope, everytime a nested if or while or for is called, we increase the scopedepth by 1

			Token f_CurrentToken;

			bool f_IsSingle = false; //used for tracking whether the class defintion we're in is a singleton or not
			bool f_ShouldShift = true;

			while (!IsENDF(f_CurrentToken) && f_ProgramTokens.size() > 0) //this loop operates on every statement that lives on scope-depth = 0, everything else is branched like a tree from the 0th level
			{
				if (!f_ShouldShift)
				{
					f_ShouldShift = true;
				}
				else
				{
					f_CurrentToken = ShiftForward(f_ProgramTokens);
					f_ProgramCounter++;
				}
				switch (f_CurrentToken.m_Type)
				{
				case TokenType::Func: //used for func and class method definitions
				{
					unique_ptr<FuncDeclaration> s_FunctionDefintion = make_unique<FuncDeclaration>();

					if (!ParseFuncDeclaration(f_CurrentScopeDepth, f_CurrentToken, f_ProgramTokens, f_Program, f_ProgramCounter, &s_FunctionDefintion))
					{
						f_ProgramTokens.clear(); //dump source code so that parsing ends immediately
						continue;
					}
					else
					{
						(f_Program)->m_ProgramFunctions.push_back(move(s_FunctionDefintion));
						//at the end of the top-level conditional flow, we will have overstepped a single token since our algorithm involves checking the next token after '}' to look for an else/else-if
						f_ShouldShift = true;

						//cout << CreateColouredText("Func Parse Success: ", "bright green") << f_CurrentToken.m_Value << "\n";
						continue;
					}
					//after parsing the function successfully we should be pointing to an already processed token, so we can safely iterate and ShiftForward() at the top of the loop
				}
				break;
				case TokenType::Single:
				{
					f_IsSingle = true; //now if a valid class syntax token is found, it will be parsed as a singleton
					//we could technically start parsing the class definition here or move the the current token without continue, but i think this is just neater for debugging
				}
				break;
				case TokenType::Class: //Class Definition, not accessor; we do that with the UserIdentifier token
				{
					//ClassDeclaration s_ClassBlock; //used for func args, var reassignment, class method access

					//if (!ParseClassDeclaration(f_CurrentScopeDepth, f_CurrentToken, f_ProgramTokens, f_ProgramCounter, s_ClassBlock, f_IsSingle))
					//{
					//	continue;
					//}
					//else
					//{
					//	f_Program.m_ProgramClasses.push_back(s_ClassBlock);
					//	f_IsSingle = false; //reset so that the next class definition doesn't get falsely labelled as a singleton
					//}
				}
				break;
				case TokenType::Struct: //define a struct
				{
					//StructDeclaration s_StructBlock; //used for func args, var reassignment, class method access

					//if (!ParseStructDeclaration(f_CurrentScopeDepth, f_CurrentToken, f_ProgramTokens, f_ProgramCounter, s_StructBlock))
					//{
					//	continue;
					//}
					//else
					//{
					//	f_Program.m_ProgramStructs.push_back(s_StructBlock);
					//}
				}
				break;
				case TokenType::ENDF:
					break;
				default:
				{
					LogAndPrint("Compiler Error: Improper grammar found in source code at line " + to_string(f_CurrentToken.m_SourceCodeLineNumber), "Parser", "error", "red");
					f_ProgramTokens.clear(); //dump all tokens, so that the compiler will stop processing the source code
				}
				break; //OwO
				}
			}
			//end of while-switch loop

			return f_Program;
		}
	};
}