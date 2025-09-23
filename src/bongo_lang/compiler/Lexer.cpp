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
        size_t f_ProgramCounter = 0;
        size_t f_CurrentLineNumber = 1;

        char f_CurrentChar;

        bool f_ShouldShift = true;
        bool f_IsCurrentlyInsideComment = false;

        while (fp_SourceCode.size() > 0)
        {
            //////////////////// Iterate Current Character ////////////////////

            if (f_ShouldShift)
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
            else if (f_IsCurrentlyInsideComment or isspace(f_CurrentChar))
            {
                continue;
            }

            //////////////////// Handle Digits or Alphabetic Characters ////////////////////

            if (isdigit(f_CurrentChar))
            {
                string f_Number = ""; // >w<

                while (fp_SourceCode.size() > 0 and isdigit(f_CurrentChar))
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

                    if (not isdigit(f_CurrentChar))
                    {
                        // Handle error: Unterminated type arrow
                        logger->Error("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer");
                        logger->Warning("Unexpected symbol following a '.' brother!, looks like you've input a non-numeric symbol while defining a decimal number", "Lexer");

                        fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                        return false;
                    }

                    while (fp_SourceCode.size() > 0 and isdigit(f_CurrentChar))
                    {
                        f_Number += f_CurrentChar;
                        f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
                        f_ProgramCounter++;
                    }
                    //push a float
                    fp_Tokens.emplace_back(f_Number, TokenType::FloatNumber, f_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
                }
                else
                {	//push an int
                    fp_Tokens.emplace_back(f_Number, TokenType::IntNumber, f_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
                }

                f_ShouldShift = false; //ensures we don't skip any crucial branch-logic for the over-stepped character
                continue; //move to next iteration
            }

            else if (isalpha(f_CurrentChar) or f_CurrentChar == '_') //used for keywords, and user identifiers like enum, class or var names
            {
                string f_Identifier = ""; //start with NOTHING

                while (fp_SourceCode.size() > 0 and (isalpha(f_CurrentChar) or f_CurrentChar == '_'))
                {
                    f_Identifier += f_CurrentChar;
                    f_CurrentChar = ShiftForward(fp_SourceCode); //shift to next character
                    f_ProgramCounter++;
                }

                if (KEYWORDS.find(f_Identifier) == KEYWORDS.end()) //if identifier is not a keyword then its just tokenized assuming its a var name or smth
                {
                    fp_Tokens.emplace_back(f_Identifier, TokenType::UserIdentifier, f_CurrentLineNumber);
                }
                else
                {
                    fp_Tokens.emplace_back(f_Identifier, KEYWORDS.at(f_Identifier), f_CurrentLineNumber);
                }

                f_ShouldShift = false; //ensures we don't skip any crucial branch-logic for the over-stepped character
                continue; //move to next iteration since
            }

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
                    fp_Tokens.emplace_back(f_CurrentStringLiteral, TokenType::StringLiteral, f_CurrentLineNumber);

                    // Shift again to move past the closing quote
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_ProgramCounter++;
                }
                else
                {
                    // Handle error: Unterminated string literal, and exit program execution
                    logger->Error("Unterminated string literal, brother! Error occured at line number: " + to_string(f_CurrentLineNumber), "Lexer");
                    fp_SourceCode.clear();
                    return false;
                }

                //IMPORTANT IF WE DONT MAKE SURE WE AVOID SHIFTING THEN EVERYTHING WILL BREAK
                f_ShouldShift = false; //just in case
                continue; //move to next iteration, we shift here because f_CurrentChar is pointing -> ' " ' 
            }
            break;
            case '=':
            {
                string f_EqualsString = "";

                while (fp_SourceCode.size() > 0 and f_CurrentChar == '=')
                {
                    f_EqualsString += f_CurrentChar;
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_ProgramCounter++;
                }

                if (f_EqualsString.size() >= 2)
                {
                    fp_Tokens.emplace_back(f_EqualsString, TokenType::StrictlyEquals, f_CurrentLineNumber);
                }
                else if (f_EqualsString.size() == 1)
                {
                    fp_Tokens.emplace_back(f_EqualsString, TokenType::Equals, f_CurrentLineNumber);
                }
                else
                {
                    //THROW ERROR
                    logger->Error("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer");
                    logger->Warning("Something bad happened involving a '=' sign brother", "Lexer");
                    fp_SourceCode.clear(); //dump source code so that lexical analysis ends immediately
                    return false;
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
                    fp_Tokens.emplace_back(s_DoesNotEqualsString, TokenType::DoesNotEquals, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else //handles the case for when nothing valid follows a '!' in the source code
                {
                    //THROW ERROR
                    logger->Error("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer");
                    logger->Warning("Something bad happened involving a '!' sign brother", "Lexer");
                    fp_SourceCode.clear(); //dump source code so that lexical analysis ends immediately
                    return false;
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
                    fp_Tokens.emplace_back(f_PlusString, TokenType::PlusEqualsOperator, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else //handles the case for when nothing valid follows a '!' in the source code
                {
                    fp_Tokens.emplace_back(f_PlusString, TokenType::AdditionOperator, f_CurrentLineNumber);
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
                    fp_Tokens.emplace_back(f_MultString, TokenType::MultEqualsOperator, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else //handles the case for when nothing valid follows a '!' in the source code
                {
                    fp_Tokens.emplace_back(f_MultString, TokenType::MultiplicationOperator, f_CurrentLineNumber);
                    f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
                    continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
                }
            }
            break;
            case '-':
            {
                string f_TypeArrow = ""; //ik whats coming next, but for conventions sake

                while (fp_SourceCode.size() > 0 and f_CurrentChar == '-')
                {
                    f_TypeArrow += f_CurrentChar;
                    f_CurrentChar = ShiftForward(fp_SourceCode);
                    f_ProgramCounter++;
                }

                if (f_CurrentChar == '>') //this goes first so that subtraction doesn't get confused with '->', since they both have only 1 dash
                {
                    f_TypeArrow += f_CurrentChar;
                    fp_Tokens.emplace_back(f_TypeArrow, TokenType::TypeArrow, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else if (f_CurrentChar == '=')
                {
                    f_TypeArrow += f_CurrentChar; //IDC THAT ITS NOT A TYPEARROW
                    fp_Tokens.emplace_back(f_TypeArrow, TokenType::MinusEqualsOperator, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else if (f_TypeArrow.size() == 1) //fuck it we ball, we deal with minus here BROTHERS
                {
                    fp_Tokens.emplace_back(f_TypeArrow, TokenType::NegativeOperator, f_CurrentLineNumber);
                    f_ShouldShift = false; // reset over-stepped character to top of lexer logical flow without shifting again to avoid missed characters
                    continue; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
                }
                else //if we have more than one consecutive '-', then it's a mistake regardless of what you were trying to do
                {
                    // Handle error: Unterminated type arrow
                    logger->Error("Error at Line Number: " + to_string(f_CurrentLineNumber), "Lexer");
                    logger->Warning("Unterminated type arrow brother!, looks like you're missing an arrow head to your type arrow definition", "Lexer");

                    fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                    return false;
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
                    fp_Tokens.emplace_back(f_DivString, TokenType::DivEqualsOperator, f_CurrentLineNumber);
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
                    fp_Tokens.emplace_back(f_DivString, TokenType::DivisionOperator, f_CurrentLineNumber);
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
                    fp_Tokens.emplace_back(f_ModString, TokenType::ModuloEqualsOperator, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else //handles the case for when nothing valid follows a '!' in the source code
                {
                    fp_Tokens.emplace_back(f_ModString, TokenType::ModulusOperator, f_CurrentLineNumber);
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
                    fp_Tokens.emplace_back(f_LesserThanString, TokenType::LesserThanOrEqual, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else //handles the case for just '<'
                {
                    fp_Tokens.emplace_back(f_LesserThanString, TokenType::LesserThan, f_CurrentLineNumber);
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
                    fp_Tokens.emplace_back(f_GreaterThanString, TokenType::GreaterThanOrEqual, f_CurrentLineNumber);
                    assert(f_ShouldShift == true);
                    continue; //just iterate as normal, and make sure the equals character isn't double counted
                }
                else //handles the case for just '>'
                {
                    fp_Tokens.emplace_back(f_GreaterThanString, TokenType::GreaterThan, f_CurrentLineNumber);
                    f_ShouldShift = false; //iterate without shifting since we've overstepped a character
                    continue;
                }
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

                logger->Error("Compiler Error: Unrecognized character found in source code at line " + to_string(f_CurrentLineNumber), "Lexer");
                fp_SourceCode.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                return false;
            }

        }

        fp_Tokens.emplace_back("", TokenType::ENDF, f_CurrentLineNumber); //label the end of the file i guess for some reason

        return true;
    }
}