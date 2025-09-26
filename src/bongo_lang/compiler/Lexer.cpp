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
#include "../../../include/bongo_lang/compiler/Lexer.h"

namespace BongoJam{

    //////////////////////////////////////////////
    // Utility Functions
    //////////////////////////////////////////////

    [[nodiscard]] char
        ShiftForward(string& fp_Src)
    {
        if (fp_Src.empty())
        {
            return '\0';
        }

        char _c = fp_Src[0];
        fp_Src.erase(fp_Src.begin());
        return _c;
    }

    [[nodiscard]] char
        Peek(const string& fp_Src)
    {
        return fp_Src[0];
    }

    //////////////////////////////////////////////
    // Tokenize Function
    //////////////////////////////////////////////

    bool
        Tokenize
        (
            string& fp_SourceCode,
            vector<Token>& fp_Tokens,
            Logger* logger
        )
    {
        size_t f_CurrentLineNumber = 1; //humans start at 1 cs reptiles start at 0 uwu

        char f_CurrentChar;

        bool f_IsCurrentlyInsideComment = false;

        while (fp_SourceCode.size() > 0)
        {
            //////////////////// Iterate Current Character ////////////////////

            f_CurrentChar = ShiftForward(fp_SourceCode);

            //////////////////// Handle Spaces, New-Lines, and Comments ////////////////////

            if (f_CurrentChar == '\n') //used to keep track of what line number we're at in the source code, we only have single line comments, so this is sufficient
            {
                f_IsCurrentlyInsideComment = false;
                f_CurrentLineNumber++;
                continue; //we can shift forwards confidently since we're currently on the newline character
            }
            else if (f_IsCurrentlyInsideComment or isspace(f_CurrentChar))
            {
                continue; //skip char since its a space or comment
            }

            //////////////////////////////////////////////////////////// Handle Digits ////////////////////////////////////////////////////////////

            if (isdigit(f_CurrentChar))
            {
                string f_Number = ""; // >w<
                f_Number += f_CurrentChar; //get current char since its a digit

                while (fp_SourceCode.size() > 0 and isdigit(Peek(fp_SourceCode)))
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
                    f_Number += f_CurrentChar;
                }

                if (Peek(fp_SourceCode) == '.') //used for handling decimal numbers eg. "let x->float = 3.14;"
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next non-numeric character
                    f_Number += f_CurrentChar; //add the decimal so we're at: "69. (rest to be parsed)" currently
                    
                    if (not isdigit(Peek(fp_SourceCode))) // Handle error: incomplete float definition -> '59. '
                    {
                        logger->Error(format("Error at Line Number: {}", f_CurrentLineNumber), "Lexer");
                        logger->Warning("Unexpected symbol following a '.' brother!, looks like you've input a non-numeric symbol while defining a decimal number", "Lexer");

                        fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                        return false;
                    }
                    //is a digit so check for digit again to verify and shiftforwards for value uwu
                    while (fp_SourceCode.size() > 0 and isdigit(Peek(fp_SourceCode)))
                    {
                        f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
                        f_Number += f_CurrentChar;
                    }
                    //push a float
                    fp_Tokens.emplace_back(f_Number, TokenType::FloatNumber, f_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
                }
                else
                {	//push an int
                    fp_Tokens.emplace_back(f_Number, TokenType::IntNumber, f_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
                }

                continue; //move to next iteration
            }

            //////////////////////////////////////////////////////////// Alphabetic Characters (outside string) ////////////////////////////////////////////////////////////

            else if (isalpha(f_CurrentChar) or f_CurrentChar == '_') //used for keywords, and user identifiers like enum, class or var names
            {
                string f_Identifier = ""; //start with NOTHING

                f_Identifier += f_CurrentChar; //grab reference to the entry char uwu

                while (fp_SourceCode.size() > 0 and (isalpha(Peek(fp_SourceCode)) or Peek(fp_SourceCode) == '_' or isdigit(Peek(fp_SourceCode))))
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
                    f_Identifier += f_CurrentChar;

                    //////////////////// Formatted Strings ////////////////////

                    if(f_CurrentChar == 'f' and Peek(fp_SourceCode) == '"')
                    {
                        f_CurrentChar = ShiftForward(fp_SourceCode); //-->'"' //shift twice since we wanna enter the string quotes

                        string f_FormattedString; //default val at ""

                        f_CurrentChar = ShiftForward(fp_SourceCode); //now pointing at hopefully a string or open bracket
                        f_FormattedString += f_CurrentChar;

                        bool f_IsEscapeCharacter = false;

                        //////////////////// Main Loop ////////////////////

                        while (fp_SourceCode.size() > 0 and f_CurrentChar != '"')
                        {
                            //////////////////// Handle Escape Characters ////////////////////

                            if (f_CurrentChar == '\\')
                            {
                                f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character

                                switch (f_CurrentChar)
                                {
                                case 'n':
                                    f_FormattedString += '\n'; // Add a newline character
                                    break;
                                case 't':
                                    f_FormattedString += '\t'; // Add a tab character
                                    break;
                                case '\\':
                                    f_FormattedString += '\\'; // Add a literal backslash
                                    break;
                                case '"':
                                    f_FormattedString += '"'; // Add a literal double quote
                                    break;
                                default:
                                    // Handle unknown escape sequences or add a fallback behavior
                                    f_FormattedString += '\\'; // Re-add the backslash as it was part of the input
                                    f_FormattedString += f_CurrentChar; // Add the unknown character as is
                                    break;
                                }
                            }
                            else if (f_CurrentChar == '{')
                            {
                                fp_Tokens.emplace_back(f_FormattedString, TokenType::FormattedStringLiteralStart, f_CurrentLineNumber);

                                f_FormattedString = ""; //reset string value for string end lexing

                                string f_StringInsert;

                                f_CurrentChar = ShiftForward(fp_SourceCode); //shift first, then while processe and shifts at bottom of loop to hit condition for '}' hopefully uwu

                                //////////////////// Parse Inserted String Value ////////////////////

                                while (fp_SourceCode.size() > 0 and f_CurrentChar != '}')
                                {
                                    if (f_CurrentChar == '"') //THROW ERROR: string terminated before formatted variable was closed
                                    {
                                        logger->Error(format("Unterminated variable inside your string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                                        fp_SourceCode.clear();
                                        return false;
                                    }

                                    f_StringInsert += f_CurrentChar;
                                    f_CurrentChar = ShiftForward(fp_SourceCode);
                                }

                                if (f_CurrentChar != '}') //THROW ERROR: string terminated before formatted variable was closed
                                {
                                    logger->Error(format("Unterminated variable inside your string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                                    fp_SourceCode.clear();
                                    return false;
                                }

                                fp_Tokens.emplace_back(f_StringInsert, TokenType::FormattedStringInsert, f_CurrentLineNumber);
                            }
                            else
                            {
                                f_FormattedString += f_CurrentChar; //proceed as usual
                            }

                            //////////////////// Shift for While-Loop ////////////////////
                            f_CurrentChar = ShiftForward(fp_SourceCode);
                        }
                        
                        // Check if we've ended on the closing quotation mark // Handle error: Unterminated string literal, and exit program execution
                        if (f_CurrentChar != '"')
                        {
                            logger->Error(format("Unterminated string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                            fp_SourceCode.clear();
                            return false;
                        }

                        // Push the final string token without the quotes
                        fp_Tokens.emplace_back(f_FormattedString, TokenType::FormattedStringLiteralEnd, f_CurrentLineNumber);
                        //move to next iteration, we shift here because f_CurrentChar is pointing -> ' " ' 
                        continue;
                    }
                }

                if (KEYWORDS.find(f_Identifier) == KEYWORDS.end()) //if identifier is not a keyword then its just tokenized assuming its a var name or smth
                {
                    fp_Tokens.emplace_back(f_Identifier, TokenType::UserIdentifier, f_CurrentLineNumber);
                }
                else
                {
                    fp_Tokens.emplace_back(f_Identifier, KEYWORDS.at(f_Identifier), f_CurrentLineNumber);
                }

                continue; //move to next iteration since
            }

            //////////////////////////////////////////////////////////// Special Tokens ////////////////////////////////////////////////////////////

            switch (f_CurrentChar)
            {
            case ';':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::SemiDot, f_CurrentLineNumber);
                break;
            case ':':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::DoubleDot, f_CurrentLineNumber);
                break;
            case '.':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::Dot, f_CurrentLineNumber);
                break;
            case ',':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::Comma, f_CurrentLineNumber);
                break;

            case '(':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::OpenParen, f_CurrentLineNumber);
                break;
            case ')':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::CloseParen, f_CurrentLineNumber);
                break;

            case '{':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::OpenBracket, f_CurrentLineNumber);
                break;
            case '}':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::CloseBracket, f_CurrentLineNumber);
                break;

            case '[':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::OpenSquareBracket, f_CurrentLineNumber);
                break;
            case ']':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::CloseSquareBracket, f_CurrentLineNumber);
                break;

            //////////////////////////////////////////////////////////// Strings / Formatted Strings ////////////////////////////////////////////////////////////

            case '"': //VERY IMPORTANT THAT WE PROCESS THIS BEFORE '/' otherwise '/' mentioned inside of strings might be ignored
            {
                string f_CurrentStringLiteral = "";

                // Shift to the next character to start capturing the string, not the opening quote
                f_CurrentChar = ShiftForward(fp_SourceCode);

                bool f_IsEscapeCharacter = false;

                //////////////////// Main Loop ////////////////////

                while (fp_SourceCode.size() > 0 and f_CurrentChar != '"')
                {
                    //////////////////// Handle Escape Characters ////////////////////

                    if (f_CurrentChar == '\\')
                    {
                        f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character

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
                        f_CurrentStringLiteral += f_CurrentChar; //proceed as usual
                    }

                    f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
                }

                // Check if we've ended on the closing quotation mark // Handle error: Unterminated string literal, and exit program execution
                if (f_CurrentChar != '"')
                {
                    logger->Error(format("Unterminated string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                    fp_SourceCode.clear();
                    return false;
                }
                
                // Push the final string token without the quotes
                fp_Tokens.emplace_back(f_CurrentStringLiteral, TokenType::StringLiteral, f_CurrentLineNumber);
                //move to next iteration, we shift here because f_CurrentChar is pointing -> ' " ' 
            }
            break;
            case '=':
            {
                string f_EqualsString = "=";

                while (fp_SourceCode.size() > 0 and Peek(fp_SourceCode) == '=') //see if more equals, if there is shift and consume uwu
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_EqualsString += f_CurrentChar;
                }

                if (f_EqualsString.size() >= 2)
                {
                    fp_Tokens.emplace_back(f_EqualsString, TokenType::StrictlyEquals, f_CurrentLineNumber);
                }
                else if (f_EqualsString.size() == 1)
                {
                    fp_Tokens.emplace_back(f_EqualsString, TokenType::Equals, f_CurrentLineNumber);
                }
                else //THROW ERROR
                {
                    logger->Error("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer");
                    logger->Warning("Something bad happened involving a '=' sign brother", "Lexer");
                    fp_SourceCode.clear(); //dump source code so that lexical analysis ends immediately
                    return false;
                }

                continue; //continue will just move the current over-stepped character back to the top of the lexer's logical flow
            }
            break;
            case '!': //'!' isn't used for not in bongojam
            {
                string sv_DoesNotEqualsString = "!";

                if (Peek(fp_SourceCode) != '=') //handles the case for when nothing valid follows a '!' in the source code //THROW ERROR
                {
                    logger->Error(format("Error at Line Number: {}, invalid token found, you used a '!' but maybe forgot a '=' after it", f_CurrentLineNumber), "Lexer");
                    logger->Warning("Something bad happened involving a '!' sign brother", "Lexer");
                    fp_SourceCode.clear(); //dump source code so that lexical analysis ends immediately
                    return false;
                }

                f_CurrentChar = ShiftForward(fp_SourceCode);
                sv_DoesNotEqualsString += f_CurrentChar;

                fp_Tokens.emplace_back(sv_DoesNotEqualsString, TokenType::DoesNotEquals, f_CurrentLineNumber);
                continue; //just iterate as normal, and make sure the equals character isn't double counted
            }
            break;
            case '+':
            {
                string f_PlusString = "+";

                if (Peek(fp_SourceCode) == '=')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator                
                    f_PlusString += f_CurrentChar;
                    fp_Tokens.emplace_back(f_PlusString, TokenType::PlusEqualsOperator, f_CurrentLineNumber);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }

                fp_Tokens.emplace_back(f_PlusString, TokenType::AdditionOperator, f_CurrentLineNumber);
                continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
            }
            break;
            case '*':
            {
                string f_MultString = "*";

                if (Peek(fp_SourceCode) == '=')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator                
                    f_MultString += f_CurrentChar;
                    fp_Tokens.emplace_back(f_MultString, TokenType::MultEqualsOperator, f_CurrentLineNumber);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }

                fp_Tokens.emplace_back(f_MultString, TokenType::MultiplicationOperator, f_CurrentLineNumber);
                continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow  
            }
            break;
            case '-':
            {
                string f_TypeArrow = "-"; //ik whats coming next, but for conventions sake

                while (fp_SourceCode.size() > 0 and Peek(fp_SourceCode) == '-')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_TypeArrow += f_CurrentChar;
                }

                if (Peek(fp_SourceCode) != '>' and Peek(fp_SourceCode) != '=') //fuck it we ball, we deal with minus here BROTHERS
                {
                    fp_Tokens.emplace_back(f_TypeArrow, TokenType::NegativeOperator, f_CurrentLineNumber);
                    continue; //start loop again or hit error u choose owo
                }

                f_CurrentChar = ShiftForward(fp_SourceCode); //look for equals or arrow tip uwu

                if (f_CurrentChar == '>') //this goes first so that subtraction doesn't get confused with '->', since they both have only 1 dash
                {
                    f_TypeArrow += f_CurrentChar;
                    fp_Tokens.emplace_back(f_TypeArrow, TokenType::TypeArrow, f_CurrentLineNumber);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else if (f_CurrentChar == '=' and f_TypeArrow.size() == 1) //check for single minus equals so '-=' not '-----=' srry m8 thats 2 far for m9
                {
                    f_TypeArrow += f_CurrentChar; //IDC THAT ITS NOT A TYPEARROW
                    fp_Tokens.emplace_back(f_TypeArrow, TokenType::MinusEqualsOperator, f_CurrentLineNumber);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                } 
                else if (f_CurrentChar == '=' and f_TypeArrow.size() > 1) //check for single minus equals so '-=' not '-----=' srry m8 thats 2 far for m9
                {
                    logger->Error(format("Error at Line Number: {}, minus equals definition has too many dashes COMON", f_CurrentLineNumber), "Lexer");
                    fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                    return false;
                }
                else //if we have more than one consecutive '-', then it's a mistake regardless of what you were trying to do // Handle error: Unterminated type arrow
                {
                    logger->Error(format("Error at Line Number: {}", f_CurrentLineNumber), "Lexer");
                    logger->Warning("Unterminated type arrow brother!, looks like you're missing an arrow head to your type arrow definition", "Lexer");
                    fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                    return false;
                }
            }
            break;
            case '/':
            {
                string f_DivString = "/";

                if (Peek(fp_SourceCode) == '=')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator
                    f_DivString += f_CurrentChar;
                    fp_Tokens.emplace_back(f_DivString, TokenType::DivEqualsOperator, f_CurrentLineNumber);

                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else if (Peek(fp_SourceCode) == '/')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //get on '/' character then start parsing comment
                    f_IsCurrentlyInsideComment = true;
                    continue;
                }

                fp_Tokens.emplace_back(f_DivString, TokenType::DivisionOperator, f_CurrentLineNumber);
                continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow   
            }
            break;
            case '%':
            {
                string f_ModString = "%";

                if (Peek(fp_SourceCode) == '=')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode); //look for an equals sign for the "+=" operator                
                    f_ModString += f_CurrentChar;
                    fp_Tokens.emplace_back(f_ModString, TokenType::ModuloEqualsOperator, f_CurrentLineNumber);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
 
                fp_Tokens.emplace_back(f_ModString, TokenType::ModulusOperator, f_CurrentLineNumber);
                continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
            }
            break;

            case '<': //idrc about inserting this into the branch above with '>'
            {
                string f_LesserThanString = "<";

                if (Peek(fp_SourceCode) == '=')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_LesserThanString += f_CurrentChar;
                    fp_Tokens.emplace_back(f_LesserThanString, TokenType::LesserThanOrEqual, f_CurrentLineNumber);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }

                //handles the case for just '<'
                fp_Tokens.emplace_back(f_LesserThanString, TokenType::LesserThan, f_CurrentLineNumber);
                continue;
            }
            break;
            case '>':
            {
                string f_GreaterThanString = ">";

                if (Peek(fp_SourceCode) == '=')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_GreaterThanString += f_CurrentChar;
                    fp_Tokens.emplace_back(f_GreaterThanString, TokenType::GreaterThanOrEqual, f_CurrentLineNumber);

                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                
                //handles the case for just '>'
                fp_Tokens.emplace_back(f_GreaterThanString, TokenType::GreaterThan, f_CurrentLineNumber);
                continue;
            }
            break;
            case '$':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::DollarSign, f_CurrentLineNumber);
                break;
            case '?':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::QuestionMark, f_CurrentLineNumber);
                break;
            case '@':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::AtSign, f_CurrentLineNumber);
                break;
            case '#':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::HashTag, f_CurrentLineNumber);
                break;
            case '&':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::Ampersand, f_CurrentLineNumber);
                break;
            default:
                logger->Error(format("Lexing Error: Unrecognized character found in source code at line: {}", f_CurrentLineNumber), "Lexer");
                fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                return false;
            }

        }

        fp_Tokens.emplace_back("END__OF__FILE", TokenType::ENDF, f_CurrentLineNumber); //label the end of the file i guess for some reason

        return true;
    }
}