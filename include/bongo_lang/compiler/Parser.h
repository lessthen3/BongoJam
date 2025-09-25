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
#pragma once

#include <cstring> // for memcpy

#include "AST.h"

#include <unordered_map>

namespace BongoJam {
    
    class Parser
    {
    private:
        shared_ptr<Logger> parser_logger = nullptr;

    public:
        Parser(shared_ptr<Logger> fp_CompilerLogger)
        {
            parser_logger = fp_CompilerLogger;
        }

        ~Parser() = default;
    private:

        bool
            IsUnaryOperator(const Token& fp_Token)
        {
            return fp_Token.m_Type == TokenType::NegativeOperator or fp_Token.m_Type == TokenType::Not;
        }

        bool
            IsTypename(const Token& fp_Token)
        {
            if (
                fp_Token.m_Type != TokenType::UserIdentifier and //used for user defined types, if this isn"t a type we"ll catch it as a runtime error
                fp_Token.m_Type != TokenType::Int and //actually refering to the type itself and not the literal value
                fp_Token.m_Type != TokenType::UnsignedInt and 
                fp_Token.m_Type != TokenType::Float and
                fp_Token.m_Type != TokenType::Double and
                fp_Token.m_Type != TokenType::String and
                fp_Token.m_Type != TokenType::Char and 
                fp_Token.m_Type != TokenType::Bool
                )
            {
                return false;
            }

            return true;
        }

        bool
            IsValue(const Token& fp_Token)
        {
            if (
                fp_Token.m_Type != TokenType::IntNumber and
                fp_Token.m_Type != TokenType::UnsignedIntNumber and
                fp_Token.m_Type != TokenType::Bool and
                fp_Token.m_Type != TokenType::StringLiteral and
                fp_Token.m_Type != TokenType::CharLiteral and
                fp_Token.m_Type != TokenType::FloatNumber and
                fp_Token.m_Type != TokenType::DoubleNumber and
                fp_Token.m_Type != TokenType::UserIdentifier //used for function calls and variables i guess idfk
                )
            {
                return false;
            }

            return true;
        }
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
                return Token("EOF", TokenType::ENDF, -1); //return escape char when source code is done being read
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

        //this method will find the smallest possible expression formed by the tokens following the number token, and return it

        //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required

        unique_ptr<Expr>
            ParseNumber
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            Token f_EntryToken = fp_CurrentToken;
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //Shift forwards to look for a semi dot or more ops

            //returning on close paren should kick start the paren chain back up to the original openparen() call
            if (fp_CurrentToken.m_Type == TokenType::SemiDot or fp_CurrentToken.m_Type == TokenType::CloseParen or fp_CurrentToken.m_Type == TokenType::Comma) //base case that stops the recursive descent
            {
                return make_unique<SingleValueExpr>(f_EntryToken); //entry token is the value, situation is . . . "string"; we're pointing at ';' rn so take the stashed entry token as val
            }

            switch (fp_CurrentToken.m_Type)
            {
                case TokenType::AdditionOperator:

                    break;
                case TokenType::MultiplicationOperator:

                    break;
                case TokenType::NegativeOperator:

                    break;
                case TokenType::DivisionOperator:

                    break;
                case TokenType::ModulusOperator:
                    break;
                case TokenType::SemiDot: //base case that stops the recursive descent

                    break;
                default:
                    parser_logger->Error(format("found : '{}', when mathematical binary operation was expected", fp_CurrentToken.m_Value), "Parser");
                    return nullptr;
            }
        }

        unique_ptr<Expr>
            ParseString //assuming the current token == StringLiteral
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            Token f_EntryToken = fp_CurrentToken;
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //Shift forwards to look for a semi dot or more ops
            
            if (fp_CurrentToken.m_Type == TokenType::SemiDot) //base case that stops the recursive descent
            {
                return make_unique<SingleValueExpr>(f_EntryToken); //entry token is the value, situation is . . . "string"; we're pointing at ';' rn so take the stashed entry token as val
            }

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //no ';' found, so expression assumed to continue

            while(fp_CurrentToken.m_Type == TokenType::CloseParen) //could be at the end of a long op ..."text"))); <-- not uncommon and definitely needs to be handled
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //at top because loop only enters if the current token is a close paren already, we look for ';' or more parens now

                if (fp_CurrentToken.m_Type == TokenType::SemiDot) //base case that stops the recursive descent
                {
                    return make_unique<SingleValueExpr>(f_EntryToken); //entry token is the value, situation is . . . "string"; we're pointing at ';' rn so take the stashed entry token as val
                }
            }

            //fall through to switch now since the expr is assumed to be  ... "text") ... <-- probably an op after the closed bracket uwu

            switch (fp_CurrentToken.m_Type)
            {
            case TokenType::AdditionOperator:
            {
                auto result = ParseAdditionExpr(fp_CurrentToken, fp_ProgramTokens);

                if (not result) //check if parse was successfull before constructing another expr
                {

                    return nullptr;
                }

                //success! parsed expr properly and signing + sealing for delivery back up the call stack

                return make_unique<BinaryOperationExpr>
                (
                    fp_CurrentToken,
                    make_unique<SingleValueExpr>(f_EntryToken),
                    move(result)
                );
            }
            break;
            default:
                parser_logger->Error(format("found : '{}', when processing : '{}' mathematical binary operation was expected", fp_CurrentToken.m_Value, f_EntryToken.m_Value), "Parser");
                fp_ProgramTokens.clear(); //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required
                return nullptr;
            }
        }

        //////////////////////////////////////////////
        // Math Operator Expressions
        //////////////////////////////////////////////

        unique_ptr<Expr>
            ParseAdditionExpr
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            switch (fp_CurrentToken.m_Type)
            {
                case TokenType::IntNumber:

                    break;
                case TokenType::UnsignedIntNumber:

                    break;
                case TokenType::FloatNumber:

                    break;
                case TokenType::DoubleNumber:

                    break;
                case TokenType::StringLiteral:

                    break;
                case TokenType::CharLiteral:

                    break;
                case TokenType::UserIdentifier: //XXX: used for function calls and variables

                    break;
                default:
                    parser_logger->Error(format("found : '{}', when mathematical expression was expected", fp_CurrentToken.m_Value), "Parser");
                    fp_ProgramTokens.clear(); //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required
                }

            return nullptr;
        }

        unique_ptr<Expr>
            ParseSubtractionExpr
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_Tokens,
                unique_ptr<Expr>* fp_Expression
            )
        {
            switch (fp_CurrentToken.m_Type)
            {
                case TokenType::AdditionOperator:

                    break;
                case TokenType::MultiplicationOperator:

                    break;
                case TokenType::NegativeOperator:

                    break;
                case TokenType::DivisionOperator:

                    break;
                case TokenType::UserIdentifier: //XXX: used for function calls

                    break;
                default:
                    parser_logger->Error(format("found : '{}', when mathematical expression was expected", fp_CurrentToken.m_Value), "Parser");
                    fp_Tokens.clear(); //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required
                    return nullptr;
                }
        }

        unique_ptr<Expr>
            ParseMultiplicationExpr
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_Tokens,
                unique_ptr<Expr>* fp_Expression
            )
        {
            switch (fp_CurrentToken.m_Type)
            {
                case TokenType::AdditionOperator:

                    break;
                case TokenType::MultiplicationOperator:

                    break;
                case TokenType::NegativeOperator:

                    break;
                case TokenType::DivisionOperator:

                    break;
                case TokenType::UserIdentifier: //XXX: used for function calls

                    break;
                default:
                    parser_logger->Error(format("found : '{}', when mathematical expression was expected", fp_CurrentToken.m_Value), "Parser");
                    fp_Tokens.clear(); //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required
                }

            return nullptr;
        }

        unique_ptr<Expr>
            ParseDivisionExpr
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_Tokens,
                unique_ptr<Expr>* fp_Expression
            )
        {
            switch (fp_CurrentToken.m_Type)
            {
                case TokenType::AdditionOperator:

                    break;
                case TokenType::MultiplicationOperator:

                    break;
                case TokenType::NegativeOperator:

                    break;
                case TokenType::DivisionOperator:

                    break;
                case TokenType::UserIdentifier: //XXX: used for function calls

                    break;
                default:
                    parser_logger->Error(format("found : '{}', when mathematical expression was expected", fp_CurrentToken.m_Value), "Parser");
                    fp_Tokens.clear(); //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required
                }

            return nullptr;
        }

        //this function is used to deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();"		    handled by ParseNumber()
        //we also deal with expressions formed within method or function calls, so this method will return an expression ending with ';' or ',' eg. myFunc(1, 3 + otherFunc(), otherFunc() * 2);
        //																																					called					      called
        unique_ptr<Expr>
            ParseUserIdentifier
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for any accessor symbols

            switch (fp_CurrentToken.m_Type)
            {
            case TokenType::Dot: //used for class calling, eg. "myClassInstance.myMethod(func args);" or "myClassInstance.myField = 69;"
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens);

                //if we arent accessing a method or field after the ".", then idc wtf u typed, that shit is getting thrown out dawg
                if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when class method was expected ", to_string(fp_CurrentToken.m_SourceCodeLineNumber), fp_CurrentToken.m_Value), "Parser");
                    parser_logger->Warning("Something bad happened while calling a class method! Make sure you're calling the proper method name", "Parser");
                    return nullptr;
                }

                //TODO: figure out how to differentiate between nested classes and methods
                while (fp_ProgramTokens.size() > 0 and fp_CurrentToken.m_Type != TokenType::SemiDot)
                {

                }


            }
            break;
            case TokenType::OpenParen: //function call
            {

            }
            break;
            case TokenType::Equals:
            {

            }
            break;
            case TokenType::StrictlyEquals:
            {

            }
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
            default:

                return nullptr;
                break;
            }
            return nullptr;
        }

        unique_ptr<Expr>
            ParseOpenParen //should always return as a full unit of "(" + ")"
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            auto f_RegularExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_RegularExpr)
            {
                parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            if (fp_CurrentToken.m_Type != TokenType::CloseParen)
            {
                parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            return move(f_RegularExpr);
        }

        //used for parsing any expr, and types it accordingly------ the assumed entry is a caller that is optimistic about the result being an expression so as a result the current token is right before said expression uwu
        unique_ptr<Expr>
            ParseRegularExpr 
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            fp_CurrentToken = ShiftForward(fp_ProgramTokens);

            switch (fp_CurrentToken.m_Type)
            {
            case TokenType::UnsignedIntNumber: //fallthrough to number
            case TokenType::FloatNumber:
            case TokenType::DoubleNumber:
            case TokenType::IntNumber:
            {
                auto sv_ParsedNumber = ParseNumber(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ParsedNumber)
                {
                    parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                return move(sv_ParsedNumber);
            }
                break;
            case TokenType::CharLiteral: //INFO: for now chars are all strings uwu idk maybe later fix that uwu
            case TokenType::StringLiteral:
            {
                auto sv_ParsedString = ParseString(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ParsedString)
                {
                    parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                return move(sv_ParsedString);
            }
            break;
            case TokenType::UserIdentifier: //XXX: used for function calls and variables
            {
                auto sv_ParsedUserIdentifier = ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ParsedUserIdentifier)
                {
                    parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                return move(sv_ParsedUserIdentifier);
            }
                break;
            case TokenType::OpenParen:
            {
                auto sv_ParsedOpenParen = ParseOpenParen(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ParsedOpenParen)
                {
                    parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                return move(sv_ParsedOpenParen);
            }
            break;
            default:
                parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }
            //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required
        }


        unique_ptr<StatementBlock>
            ParseStatementBlock
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens,
                bool fp_IsInLoop
            )
        {
            unique_ptr<StatementBlock> f_StatementBloc = make_unique<StatementBlock>(); //bloc cause so im so random >w< rawr *wags tail

            fp_CurrentToken = ShiftForward(fp_ProgramTokens);

            if (fp_CurrentToken.m_Type != TokenType::OpenBracket) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unrecognized symbol found when parsing statement block! Try taking a look at your '{ }' code-body wrappers", "Parser");
                return nullptr;
            }

            //parse the else code body
            while (fp_ProgramTokens.size() > 0)
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic   

                switch (fp_CurrentToken.m_Type)
                {
                case TokenType::Return:
                {
                }
                break;
                case TokenType::Break:
                {
                    if (not fp_IsInLoop) //THROW ERROR
                    {
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        parser_logger->Warning("You are not allowed to use a break statement outside of a loop brother!", "Parser");
                        return nullptr;
                    }

                    f_StatementBloc->CodeBody.emplace_back(make_unique<BreakStatement>());
                }
                break;
                case TokenType::Continue:
                {
                    if (not fp_IsInLoop) //THROW ERROR
                    {
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        parser_logger->Warning("You are not allowed to use a continue statement outside of a loop brother!", "Parser");
                        return nullptr;
                    }

                    f_StatementBloc->CodeBody.emplace_back(make_unique<ContinueStatement>());
                }
                break;
                case TokenType::If:
                {
                    unique_ptr<IfDeclaration> sv_NestedIfStatement = ParseIfBlock(fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_NestedIfStatement)
                    {
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        return nullptr;
                    }

                    f_StatementBloc->CodeBody.push_back(move(sv_NestedIfStatement));
                }
                break;
                default:
                    parser_logger->Error(format("Error at Line Number: {}, found : '{}', when statement was expected inside a code block", to_string(fp_CurrentToken.m_SourceCodeLineNumber), fp_CurrentToken.m_Value), "Parser");
                    return nullptr;
                }
            }

            if (fp_CurrentToken.m_Type != TokenType::CloseBracket) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser");
                return nullptr;
            }

            return move(f_StatementBloc);
        }

        //////////////////////////////////////////////
        // Conditional Logic Blocks
        //////////////////////////////////////////////

        unique_ptr<IfDeclaration>
            ParseIfBlock
            (
                bool fp_IsInLoop, 
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<IfDeclaration> f_IfDec = make_unique<IfDeclaration>();

            size_t f_IfLineNumber = fp_CurrentToken.m_SourceCodeLineNumber;

            fp_CurrentToken = ShiftForward(fp_ProgramTokens);
            
            if (fp_CurrentToken.m_Type != TokenType::OpenParen)
            {
                //THROW ERROR
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your if statement brother! Try looking at your brackets on the if-statement", "Parser");
                return nullptr;
            }
            
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shiftforwards to find a valid value entry including user identifiers
            
            if (not (IsValue(fp_CurrentToken) or IsUnaryOperator(fp_CurrentToken))) //did not find a valid entry
            {
                //THROW ERROR
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your if statement brother! Try looking at your brackets on the if-statement", "Parser");
                return nullptr;
            }

            //parse through the condition ' if "( cond )" '

            auto f_ConditionExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_ConditionExpr)
            {

                return nullptr;
            }

            f_IfDec->m_Condition = move(f_ConditionExpr);

            //assume shift forward assuming that the last token should be the end of the expression still so if(..."text") <-- current token should = ')' rn since the close paren shouldnt parse 

            if (fp_CurrentToken.m_Type != TokenType::CloseParen)
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                return nullptr;
            }

            //fp_CurrentToken = ')' right now
            //assuming that when the bracket count goes to 0, we are outside of the if statement condition, we now check for the first curly-brace
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now to look for an open bracket
            

            if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
            {
                //THROW ERROR
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser");
                return nullptr;
            }

            //parse the if-statement code body
            while (fp_ProgramTokens.size() > 0)
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic

                switch (fp_CurrentToken.m_Type)
                {
                case TokenType::Return:
                {
                }
                break;
                case TokenType::Break:
                {
                    if (not fp_IsInLoop)
                    {
                        //THROW ERROR
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        parser_logger->Warning("You are not allowed to use a break statement outside of a loop brother!", "Parser");
                        return nullptr;
                    }

                    f_IfDec->CodeBody.emplace_back(make_unique<BreakStatement>()); //aparently its easier to use emplace back since its the same as push back but i can access out of it since ++17 uwu
                }
                break;
                case TokenType::If:
                {
                    auto sv_NestedIfStatement = ParseIfBlock(fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_NestedIfStatement)
                    {
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        return nullptr;
                    }

                    f_IfDec->m_ElseIfStatements.push_back(move(sv_NestedIfStatement));
                }
                break;
                default:
                    break;
                }
            }

            if (fp_CurrentToken.m_Type == TokenType::ENDF)
            {
                parser_logger->Error(format("Found end of file instead of the end of if declaration at line: {}", f_IfLineNumber), "Parser");
                return nullptr;
            }

            if (fp_CurrentToken.m_Type != TokenType::CloseBracket)
            {
                //THROW ERROR
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser");
                return nullptr;
            }

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic
            
            //idk wtf i had here before but yeah keep looping while else if's are found uwu
            while (fp_CurrentToken.m_Type == TokenType::Elif)
            {
                auto sv_FallThroughCondition = ParseIfBlock(fp_IsInLoop, fp_CurrentToken, fp_ProgramTokens);

                if (not sv_FallThroughCondition)
                {
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    return nullptr;
                }

                f_IfDec->m_ElseIfStatements.push_back(move(sv_FallThroughCondition));
            }

            if (fp_CurrentToken.m_Type == TokenType::Else)
            {
                auto f_ElseStatement = ParseElseBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

                if (not f_ElseStatement)
                {
                    parser_logger->Error(format("Error while defining an else statement at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    return nullptr;
                }

                f_IfDec->m_ElseStatement = move(f_ElseStatement);
            }

            return move(f_IfDec);
        }

        unique_ptr<ElseDeclaration>
            ParseElseBlock
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens,
                bool fp_IsInLoop
            )
        {
            Token f_EntryToken = fp_CurrentToken;

            unique_ptr<ElseDeclaration> f_ElseDec = make_unique<ElseDeclaration>();

            auto f_StatementBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

            if (not f_StatementBlock)
            {
                parser_logger->Error(format("Error occured at line: '{}' while trying to parse your else statement >:^(", to_string(f_EntryToken.m_SourceCodeLineNumber)), "Parser");
                return nullptr;
            }

            f_ElseDec->CodeBody = move(f_StatementBlock->CodeBody); //move codebody over idk kinda lazy but whatever fits the ptr return error structure better maybe ptr -> vec but idk eh

            return move(f_ElseDec);
        }

        WhileLoopDeclaration
            ParseWhileBlock()
        {

        }

        ForLoopDeclaration
            ParseForBlock()
        {

        }

        unique_ptr<ReturnStatement>
            ParseReturnStatement
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            Token f_EntryToken = fp_CurrentToken;

            unique_ptr<ReturnStatement> f_ReturnStatement = make_unique<ReturnStatement>();

            auto f_Expr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_Expr)
            {
                parser_logger->Error(format("Error at Line Number: {}, unable to parse return expression", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            return move(f_ReturnStatement);
        }

        unique_ptr<FuncDeclaration>
            ParseFuncDeclaration
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<FuncDeclaration> f_FuncDec = make_unique<FuncDeclaration>();

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //assuming this is being called when fp_CurrentToken == TokenType::Func
            
            if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your function brother! Try taking a look at your function name definition", "Parser");
                return nullptr;
            }

            f_FuncDec->m_FuncName = fp_CurrentToken; //record the function name
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren

            if (fp_CurrentToken.m_Type != TokenType::OpenParen) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your function brother! Try taking a look at how you've placed your parenthesis", "Parser");
                return nullptr;
            }

            //if it"s an open paren then we"re clear to move forward and read the condition inside
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren

            FuncArgument f_CurrentArgument;

            while (1)
            {
                if (fp_CurrentToken.m_Type == TokenType::CloseParen) //handles variables, function, and class instance names being passed as a single argument, could use this for some semi-dynamic typing xdxd
                {
                    break;
                }

                //handles variables, function, and class instance names being passed as a single argument, could use this for some semi-dynamic typing xdxd
                if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR 
                {
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Unrecognized symbol following an open parenthesis while declaring: " + f_FuncDec->m_FuncName.m_Value + "'s arguments brother! Try taking a look at your function argument(s) defintion", "Parser");
                    return nullptr;
                }

                f_CurrentArgument.Name = fp_CurrentToken;
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
                
                if (fp_CurrentToken.m_Type != TokenType::TypeArrow)
                {
                    //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Unrecognized symbol following a name definition while declaring: " + f_FuncDec->m_FuncName.m_Value + "'s arguments brother! Try taking a look at your type-arrows", "Parser");
                    return nullptr;
                }

                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
                

                if (not IsTypename(fp_CurrentToken))
                {
                    //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Unrecognized type found while declaring: " + f_FuncDec->m_FuncName.m_Value + "'s brother! Are you sure you've entered a valid type in " + f_FuncDec->m_FuncName.m_Value + "'s argument definition?", "Parser");
                    return nullptr;
                }

                f_CurrentArgument.Type = fp_CurrentToken; //type indices correspond to the func arg list
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for default value or another argument

                //////////////////////////////////////////////////////////// Search for default value declaration ////////////////////////////////////////////////////////////
                if (fp_CurrentToken.m_Type == TokenType::Equals) //default values have to be literals, since its calculated at compile time, so no runtime values
                {
                    fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for literal default value
                    
                    if (not IsValue(fp_CurrentToken)) //THROW ERROR
                    {
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        parser_logger->Warning(format("Expected a value after function argument default value declaration, found '{}'!", f_FuncDec->m_FuncName.m_Value), "Parser");
                        //could add some debug functions here to query the found token, and make a guess at what was intended uwu, tahts for bongo or wait compilerdebugtools uwu
                        return nullptr;
                    }

                    //TODO: allow static, const compile time variables to be used for default values but not rn uwu
                    //no need to parse an expr since it should always be a literal value no runtime bs for compile time functions
                    f_CurrentArgument.DefaultValue = fp_CurrentToken; //type indices correspond to the func arg list 
                }

                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for comma or close paren

                if (fp_CurrentToken.m_Type == TokenType::Comma)
                {
                    f_FuncDec->Arguments.push_back(f_CurrentArgument);
                    f_CurrentArgument = FuncArgument(); //zero out struct
                    continue; //search for more arguments
                }
                else  if (fp_CurrentToken.m_Type == TokenType::CloseParen)
                {
                    f_FuncDec->Arguments.push_back(f_CurrentArgument); //push last valid argument no need to zero out argument var
                    break;
                }
                else //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning(format("Unexpected symbol found in: {}'s argument defintion! \n Try taking a look at your comma separation between function parameters", f_FuncDec->m_FuncName.m_Value), "Parser");
                    return nullptr;
                }
            } //end of function arg parsing

            ////////////////////////////////////////// idk
            //if function definition was valid, then f_CurrentToken = ')' right now
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for a type arrow definition
            
            if (fp_CurrentToken.m_Type != TokenType::TypeArrow) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unexpected symbol found in: " + f_FuncDec->m_FuncName.m_Value + "'s return type defintion! \n Try taking a look at your type-arrow definition", "Parser");
                return nullptr;
            }

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for a return type definition

            if (not IsTypename(fp_CurrentToken)) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Invalid type found in: " + f_FuncDec->m_FuncName.m_Value + "'s return type defintion! \n Try taking a look at your type-arrow definition", "Parser");
                return nullptr;
            }
 
            f_FuncDec->m_FuncReturnType = fp_CurrentToken; //assign return type to the FuncDeclaration struct
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open bracket to indicate function body code    

            if (fp_CurrentToken.m_Type != TokenType::OpenBracket) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unrecognized symbol following " + f_FuncDec->m_FuncName.m_Value + "'s function parameter definition! Try taking a look at your function body definition", "Parser");
                return nullptr;
            }

            //we don't shift here because we want the while loop to always shift at the top, so we just leave it for the first iteration uwu

            bool f_IsDoneParsingCodeBody = true;
            //parse function body definition
            while (f_IsDoneParsingCodeBody)
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to start parsing the function body

                Token f_EntryToken = fp_CurrentToken; //used for debugging

                switch (fp_CurrentToken.m_Type)
                {
                case TokenType::Print:
                {
                    unique_ptr<PrintFunction> sv_PrintFuncCall = ParsePrintFunction(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_PrintFuncCall)
                    {
                        parser_logger->Error(format("Parsing Error at line: {}, in function named : '{}' invalid call to print() function", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
                        return nullptr;
                    }
                    
                    f_FuncDec->CodeBody.push_back(move(sv_PrintFuncCall));
                    //current token should be ';' should be safe to shift 
                    //after parsing the print function successfully we should be pointing to an already processed token, so we can safely iterate and ShiftForward() at the top of the loop
                }
                break;
                case TokenType::UserIdentifier: //deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();" specifically
                {
                    auto sv_UserDefinedAction = ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens); //used for var reassignment, class method access

                    if (not sv_UserDefinedAction)
                    {
                        parser_logger->Error(format("Parsing Error at line: {}, bad grammer found involving a user identifier in function named: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
                        return nullptr;
                    }

                    f_FuncDec->CodeBody.push_back(move(sv_UserDefinedAction));
                    //current token should be ';' should be safe to shift
                }
                break;
                case TokenType::OpenBracket: //we want to handle scope declarations first since any keywords will overshadow it in the logical flow
                {
                    unique_ptr<ScopeDeclaration> sv_ScopeBlock = ParseScopeDeclaration(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_ScopeBlock)
                    {
                        parser_logger->Error(format("Parsing Error at line: {}, invalid scope declaration found inside function with name: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
                        return nullptr;
                    }

                    f_FuncDec->CodeBody.push_back(move(sv_ScopeBlock));
                    //current token should be '}' should be safe to shift  
                }
                break;
                case TokenType::Var:
                {
                    auto sv_VariableDefinition = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_VariableDefinition)
                    {
                        parser_logger->Error(format("Parsing Error at line: {}, invalid variable definition found inside function named: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
                        return nullptr;
                    }

                    f_FuncDec->CodeBody.push_back(move(sv_VariableDefinition));
                    //current token should be ';' should be safe to shift
                }
                break;
                case TokenType::If:
                {
                    auto sv_IfStatement = ParseIfBlock(false, fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_IfStatement)
                    {
                        parser_logger->Error(format("Parsing Error at line: {}, invalid if statement found inside function named: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
                        return nullptr;
                    }

                    f_FuncDec->CodeBody.push_back(move(sv_IfStatement));
                }
                break;
                case TokenType::While:
                    break;
                case TokenType::For:
                    break;
                case TokenType::Try:
                    break;
                case TokenType::Return:
                {
                    auto sv_ReturnStatement = ParseReturnStatement(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_ReturnStatement)
                    {
                        parser_logger->Error(format("Parsing Error at line: {}, invalid return statement found inside function named: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
                        return nullptr;
                    }

                    f_FuncDec->CodeBody.push_back(move(sv_ReturnStatement));
                }
                    break;
                case TokenType::CloseBracket:
                    f_IsDoneParsingCodeBody = false; //eh best solution i got for now uwu
                    break;
                default:
                    parser_logger->Error(format("Parsing Error at line: {}, found : '{}', when statement was expected inside function named: '{}'", f_EntryToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value, f_FuncDec->m_FuncName.m_Value), "Parser");
                    return nullptr;
                }
            }

            //since we successfully parsed the function body, due to our while-loop condition, f_CurrentToken = '}',
            //we don't need to ShiftForward() here because when this function exits, it leaves that responsibility up to the while loop that called it
            //so fp_CurrentToken = '}' aka close bracket

            if (f_FuncDec->m_FuncReturnType.m_Type != TokenType::Void and (not ValidateFunctionReturnPaths())) //validate every path returns a value for non-void funcs
            {

                return nullptr;
            }

            return move(f_FuncDec);
        }

        bool
            ValidateFunctionReturnPaths()
        {

            return true;
        }

        unique_ptr<FunctionCallExpr>
            ParseFunctionCall
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<FunctionCallExpr> f_FuncCallExpr = make_unique<FunctionCallExpr>();

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //assuming we're being called from 
             
            if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR
            {
               parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your function brother! Try taking a look at your function name definition", "Parser");
                return nullptr;
            }

            f_FuncCallExpr->m_FuncName = fp_CurrentToken; //record the function name
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
                
            if (fp_CurrentToken.m_Type != TokenType::OpenParen) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your function brother! Try taking a look at how you've placed your parenthesis", "Parser");
                return nullptr;
            }


            //FUNCTION ARGUMENTS
            while (fp_ProgramTokens.size() > 0)
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren

                switch (fp_CurrentToken.m_Type)
                {
                case TokenType::CloseParen:
                    continue; // I THINK THIS WORKS THE WAY I WANT IT TO
                case TokenType::OpenParen:
                    continue;
                case TokenType::UserIdentifier: //handles variables, function, and class instance names being passed as a single argument
                {
                    auto sv_UserDefinedAction = ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens); //used for var reassignment, class method access

                    if (not sv_UserDefinedAction)
                    {
                        return nullptr;
                    }

                    //f_FuncCallExpr->
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
                default: //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Something bad happened while calling a class method! Make sure you're calling the proper method name", "Parser");
                    return nullptr;
                }
                break;
                }
            }

            return move(f_FuncCallExpr);
        }

        bool
            ParseClassDeclaration
            (

                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens, 

                ClassDeclaration& fp_ClassBlock, 
                bool fp_IsSingle
            )
        {
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an user defined identifier eg. "class MyClass"
            

            if (fp_CurrentToken.m_Type == TokenType::UserIdentifier)//if it"s a proper name defintion then we can now proceed forwards
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren
                

                if (fp_CurrentToken.m_Type != TokenType::OpenBracket)
                {
                    //THROW ERROR
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser");
                    return false;
                }

                while (fp_ProgramTokens.size() > 0 && fp_CurrentToken.m_Type != TokenType::CloseBracket)
                {
                    fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward since the current character is "{"
                    
                }
            }
            else
            {
                //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser");
                return false;
            }

            return true;
        }

        unique_ptr<StructDeclaration>
            ParseStructDeclaration
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<StructDeclaration> f_StructDec = make_unique<StructDeclaration>();

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an user defined identifier eg. "class MyClass"
            
            if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)//if it"s a proper name defintion then we can now proceed forwards, //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your struct brother, did you forget or type your name wrong?", "Parser");
                return nullptr;
            }

            f_StructDec->StructName = fp_CurrentToken; //since the token is a user identifier it must be the name uwu
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for an open paren

            if (fp_CurrentToken.m_Type != TokenType::OpenBracket) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser");
                return nullptr;
            }

            while (fp_ProgramTokens.size() > 0 and fp_CurrentToken.m_Type != TokenType::CloseBracket)
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward since the current character is "{"

                switch (fp_CurrentToken.m_Type)
                {
                case TokenType::Var: //parse field declaration
                {
                    auto sv_VariableDeclaration = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_VariableDeclaration)
                    {
                        parser_logger->Error(format("Failed to parse variable declaration inside struct : '{}' at line number: {} ", f_StructDec->StructName.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                        return nullptr;
                    }

                    f_StructDec->Members.push_back(move(sv_VariableDeclaration));
                }
                break;
                default:
                    parser_logger->Error(format("Error at Line Number: {}, found : '{}', when declaring variables inside a struct", to_string(fp_CurrentToken.m_SourceCodeLineNumber), fp_CurrentToken.m_Value), "Parser");
                    return nullptr;
                }
            }

            return move(f_StructDec);
        }

        unique_ptr<ScopeDeclaration>
            ParseScopeDeclaration
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<ScopeDeclaration> f_ScopeDec = make_unique<ScopeDeclaration>();

            return move(f_ScopeDec);
        }

        ListDeclaration
            ParseListDeclaration()
        {

        }

        DictionaryDeclaration
            ParseDictionaryDeclaration()
        {

        }

        unique_ptr<VarDeclaration> //returns true if it worked, false if it failed
            ParseVarDeclaration
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<VarDeclaration> f_VarDec = make_unique<VarDeclaration>();

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for any accessor symbols

            if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
            {
                //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring a variable brother! Try taking a look at the variable name definition", "Parser");
                return nullptr;
            }
            
            f_VarDec->Name = fp_CurrentToken;
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
                        

            if (fp_CurrentToken.m_Type != TokenType::TypeArrow)
            {
                //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Invalid symbol found after name-definition while declaring a variable brother! Try taking a look at the variable type-arrow definition", "Parser");
                return nullptr;
            }

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for type and member access level
            

            if (not IsTypename(fp_CurrentToken))
            {
                //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Something bad happened while declaring a variable brother! Try taking a look at the variable type definition", "Parser");
                return nullptr;
            }

            f_VarDec->Type = fp_CurrentToken;
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
            

            switch(fp_CurrentToken.m_Type)
            {
                case TokenType::Equals:
                {

                }
                break;
                case TokenType::ModuloEqualsOperator:
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
                    //THROW ERROR
                        parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Unexpected symbol found after type-indicator while declaring a variable brother! Try taking a look at the variable type definition", "Parser");
                    return nullptr;
            }

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
            
            auto f_ParsedRegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if(not f_ParsedRegExpr)
            {
                //THROW ERROR
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Invalid value found while declaring a variable brother! Try taking a look at the variable value definition", "Parser");
                return nullptr;
            }
            
            //now can move value expr into default value since if no compatible result is found, the function exits early with nullptr signaling failed parse
            f_VarDec->DefaultValue = move(f_ParsedRegExpr);

            return move(f_VarDec);
        }

        unique_ptr<StatementNode>
            ParseStatic
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {
            //Shift forward to look for a type token
            fp_CurrentToken = ShiftForward(fp_ProgramTokens);

            switch (fp_CurrentToken.m_Type)
            {
                case TokenType::Var:
                {
                    auto f_StaticVarDec = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

                    if (not f_StaticVarDec)
                    {
                        parser_logger->Error
                        (
                            format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)),
                            "Parser"
                        );

                        return nullptr;
                    }

                    f_StaticVarDec->IsStatic = true;

                    return move(f_StaticVarDec);
                }
                break;
                case TokenType::Func:
                {
                    auto sv_StaticFuncDec = ParseFuncDeclaration(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_StaticFuncDec)
                    {
                        parser_logger->Error
                        (
                            format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)),
                            "Parser"
                        );

                        return nullptr;
                    }

                    sv_StaticFuncDec->IsStatic = true;

                    return move(sv_StaticFuncDec);
                }
                break;
                case TokenType::Const: //parse as regular var and just add const decorator on top of static
                {
                    auto sv_StaticConstVarDec = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

                    if (not sv_StaticConstVarDec)
                    {
                        parser_logger->Error
                        (
                            format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)),
                            "Parser"
                        );

                        return nullptr;
                    }

                    sv_StaticConstVarDec->IsStatic = true;
                    sv_StaticConstVarDec->IsConst = true;

                    return move(sv_StaticConstVarDec);
                }
                break;
                default:
                    parser_logger->Error
                    (
                        format("Parsing Error: expected var when using static, eg . 'static var ~~', but found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)),
                        "Parser"
                    );
            }

            return nullptr; // :(
        }

        unique_ptr<StatementNode>
            ParseConstant
            (
                Token& fp_CurrentToken,
                vector<Token>& fp_ProgramTokens
            )
        {

            return nullptr;
        }

        //////////////////////////////////////////////
        // Parse Standard Library Functions
        //////////////////////////////////////////////

        unique_ptr<PrintFunction>
            ParsePrintFunction
            (
                Token& fp_CurrentToken, 
                vector<Token>& fp_ProgramTokens
            )
        {
            unique_ptr<PrintFunction> f_PrintFunc = make_unique<PrintFunction>();

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for '('   

            if (fp_CurrentToken.m_Type != TokenType::OpenParen) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unexpected symbol found when '(' was expected during a print() call!", "Parser");
                return nullptr;
            }
            
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a string token
                
            if (fp_CurrentToken.m_Type != TokenType::StringLiteral) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Tried to pass a non-text data type when text was expected! Try taking a look at your print() call argument", "Parser");
                return nullptr;
            }
            
            f_PrintFunc->m_FuncArgs[0] = (make_unique<SingleValueExpr>(fp_CurrentToken)); //index 0 for the first arg of print
            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow    
            
            //check for an optional colour parameter
            if (fp_CurrentToken.m_Type == TokenType::Comma)
            {
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward to look for another string literal that indicates the colour
                
                if (fp_CurrentToken.m_Type != TokenType::StringLiteral) //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                    parser_logger->Warning("Tried to pass a non-text data type when text was expected! Try taking a look at your print() call colour argument", "Parser");
                    return nullptr;
                }
                
                f_PrintFunc->m_FuncArgs[1] = make_unique<SingleValueExpr>(fp_CurrentToken); //colour arg, so the 1st index
                fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a type arrow
            }

            //we overstep a token if the above arguments for print are valid, so no need to shift again

            if (fp_CurrentToken.m_Type != TokenType::CloseParen) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unexpected symbol found when ')' was expected during a print() call!", "Parser");
                return nullptr;
            }

            fp_CurrentToken = ShiftForward(fp_ProgramTokens); //shift forward one token to check for a string token

            if (fp_CurrentToken.m_Type != TokenType::SemiDot) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                parser_logger->Warning("Unexpected symbol found when ';' was expected after a call to print()", "Parser");
                return nullptr;
            }

            //we let the main while loop ShiftForward() off of the ';'
            return move(f_PrintFunc);
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

        unique_ptr<Program>
            ConstructAST(vector<Token>& fp_ProgramTokens)
        {
            map<string, uint32_t> f_VariableToScopeMap;

            unique_ptr<Program> f_Program = make_unique<Program>();

            Token f_CurrentToken;
            Token f_EntryToken;

            bool f_IsSingle = false; //used for tracking whether the class defintion we're in is a singleton or not
            bool f_ShouldShift = true;

            while (not IsENDF(f_CurrentToken) and fp_ProgramTokens.size() > 0) //this loop operates on every statement that lives on scope-depth = 0, everything else is branched like a tree from the 0th level
            {
                if (not f_ShouldShift)
                {
                    f_ShouldShift = true;
                }
                else
                {
                    f_CurrentToken = ShiftForward(fp_ProgramTokens);
                }
                switch (f_CurrentToken.m_Type)
                {
                case TokenType::Func: //used for func and class method definitions
                {
                    f_EntryToken = f_CurrentToken;
                    unique_ptr<FuncDeclaration> sv_FunctionDefintion = ParseFuncDeclaration(f_CurrentToken, fp_ProgramTokens);

                    if (not sv_FunctionDefintion.get())
                    {
                        parser_logger->Error(format("Parsing Error at line:'{}', function named : '{}' declaration is invalid", f_EntryToken.m_SourceCodeLineNumber, "main uwu"), "Parser");
                        return nullptr;
                    }
                    
                    f_Program->ParsedScript.push_back(move(sv_FunctionDefintion));

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

                    //if (not ParseClassDeclaration(f_CurrentToken, f_ProgramTokens, s_ClassBlock, f_IsSingle))
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

                    //if (not ParseStructDeclaration(f_CurrentToken, f_ProgramTokens, s_StructBlock))
                    //{
                    //	continue;
                    //}
                    //else
                    //{
                    //	f_Program.m_ProgramStructs.push_back(s_StructBlock);
                    //}
                }
                break;
                case TokenType::NameSpace:
                {
                    unique_ptr<NameSpaceBlock> sv_NameSpace = make_unique<NameSpaceBlock>();

                }
                break;
                case TokenType::Include:
                {
                    unique_ptr<IncludeStatement> sv_Include = make_unique<IncludeStatement>();
                    //Shift forward to look for a string token
                    f_CurrentToken = ShiftForward(fp_ProgramTokens);

                    if (f_CurrentToken.m_Type != TokenType::StringLiteral)
                    {
                        parser_logger->Error
                        (
                            format("Parsing Error: expected string for include but found: '{}' instead in source code at line: {}", f_CurrentToken.m_Value, to_string(f_CurrentToken.m_SourceCodeLineNumber)),
                            "Parser"
                        );

                        return nullptr;
                    }

                    sv_Include->m_IncludePath = f_CurrentToken;
                    f_Program->ParsedScript.push_back(move(sv_Include));

                    continue; //should shiftforward at top of loop at work fine uwu
                }
                break;
                case TokenType::Var:
                {
                    unique_ptr<VarDeclaration> sv_GlobalVariable = ParseVarDeclaration(f_CurrentToken, fp_ProgramTokens);

                    if (not sv_GlobalVariable)
                    {
                        parser_logger->Error
                        (
                            format("Parsing Error: Improper grammar used when defining global variable, found: '{}' instead in source code at line: {}", f_CurrentToken.m_Value, to_string(f_CurrentToken.m_SourceCodeLineNumber)),
                            "Parser"
                        );
                    }

                    f_Program->ParsedScript.push_back(move(sv_GlobalVariable)); //global var owo
                }
                break;
                case TokenType::Static:
                {
                    auto sv_StaticStatement = ParseStatic(f_CurrentToken, fp_ProgramTokens);

                    if (not sv_StaticStatement)
                    {

                        return nullptr;
                    }

                    f_Program->ParsedScript.push_back(move(sv_StaticStatement)); //global static var
                }
                break; //should shiftforward at top of loop at work fine uwu
                case TokenType::Const:
                {
                    unique_ptr<StatementNode> sv_ConstantStatement = ParseConstant(f_CurrentToken, fp_ProgramTokens);

                    if (not sv_ConstantStatement)
                    {

                        return nullptr;
                    }

                    f_Program->ParsedScript.push_back(move(sv_ConstantStatement)); //global const var
                }
                break;
                case TokenType::ENDF:
                    break;
                default:
                {
                    parser_logger->Error
                    (
                        format("Compiler Error: Improper grammar used! found: '{}' in source code at line: {} ", f_CurrentToken.m_Value, to_string(f_CurrentToken.m_SourceCodeLineNumber)),
                        "Parser"
                    );

                    return nullptr;
                }
                break; //OwO
                }
            }
            //end of while-switch loop

            return move(f_Program);
        }
    };
}