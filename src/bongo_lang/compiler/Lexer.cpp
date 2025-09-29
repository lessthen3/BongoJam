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

namespace BongoJam {

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
        if (fp_Src.empty())
        {
            return '\0';
        }

        return fp_Src[0];
    }

    [[nodiscard]] Token
        LexNumber
        (
            string& fp_Src,
            char& fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber,
            Logger* logger
        )
    {
        string f_Number = ""; // >w<

        f_Number += fp_CurrentChar; //get current char since its a digit

        while (fp_Src.size() > 0 and isdigit(Peek(fp_Src)))
        {
            fp_CurrentChar = ShiftForward(fp_Src); //shift to next character
            f_Number += fp_CurrentChar;
        }

        if (Peek(fp_Src) == '.') //used for handling decimal numbers eg. "let x->float = 3.14;"
        {
            fp_CurrentChar = ShiftForward(fp_Src); //shift to next non-numeric character
            f_Number += fp_CurrentChar; //add the decimal so we're at: "69. (rest to be parsed)" currently

            if (not isdigit(Peek(fp_Src))) // Handle error: incomplete float definition -> '59. '
            {
                logger->Error(format("Error at Line Number: {}, unexpected symbol: '{}' found following a '.' brother!, looks like you've input a non-numeric symbol while defining a decimal number", fp_CurrentLineNumber, fp_CurrentChar), "Lexer");
                fp_Src.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                return Token();
            }
            //is a digit so check for digit again to verify and shiftforwards for value uwu
            while (fp_Src.size() > 0 and isdigit(Peek(fp_Src)))
            {
                fp_CurrentChar = ShiftForward(fp_Src); //shift to next character
                f_Number += fp_CurrentChar;
            }

            //push a float
            return Token(f_Number, TokenType::FloatNumber, fp_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
        }

        //push an int by default because otherwise the float would return uwu
        return Token(f_Number, TokenType::IntNumber, fp_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
    }

    [[nodiscard]] string //assumed being called within if(curent char is alpha) { LexWord(...)}
        LexWord
        (
            string& fp_Src,
            char& fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber
        )
    {
        string f_Word;

        f_Word += fp_CurrentChar; //add current character since its already on one

        while ((isalpha(Peek(fp_Src)) or isdigit(Peek(fp_Src)) or Peek(fp_Src) == '_') and not fp_Src.empty())//used for tracking identifier like idk PeachNode2D uwu
        {
            fp_CurrentChar = ShiftForward(fp_Src);
            f_Word += fp_CurrentChar;
        }

        return f_Word;
    } //this func doens't really throw errors since it just tracks alpha chars uwu and exits when at EOF

    [[nodiscard]] bool
        LexOperator
        (
            string& fp_Src,
            char& fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber,
            bool& fp_IsCurrentlyInsideComment,
            Logger* logger
        )
    {
        switch (fp_CurrentChar)
        {
        case '=':
        {
            string f_EqualsString = "=";

            while (fp_Src.size() > 0 and Peek(fp_Src) == '=') //see if more equals, if there is shift and consume uwu
            {
                fp_CurrentChar = ShiftForward(fp_Src);
                f_EqualsString += fp_CurrentChar;
            }

            if (f_EqualsString.size() >= 2)
            {
                fp_ProgramTokens.emplace_back(f_EqualsString, TokenType::StrictlyEquals, fp_CurrentLineNumber);
            }
            else if (f_EqualsString.size() == 1)
            {
                fp_ProgramTokens.emplace_back(f_EqualsString, TokenType::Equals, fp_CurrentLineNumber);
            }
            else //THROW ERROR
            {
                logger->Error("Error at Line Number: " + to_string(fp_CurrentLineNumber), "Lexer");
                logger->Warning("Something bad happened involving a '=' sign brother", "Lexer");
                fp_Src.clear(); //dump source code so that lexical analysis ends immediately
                return false;
            }

            break; //continue will just move the current over-stepped character back to the top of the lexer's logical flow
        }
        break;
        case '!': //'!' isn't used for not in bongojam
        {
            string sv_DoesNotEqualsString = "!";

            if (Peek(fp_Src) != '=') //handles the case for when nothing valid follows a '!' in the source code //THROW ERROR
            {
                logger->Error(format("Error at Line Number: {}, invalid token found, you used a '!' but maybe forgot a '=' after it", fp_CurrentLineNumber), "Lexer");
                fp_Src.clear(); //dump source code so that lexical analysis ends immediately
                return false;
            }

            fp_CurrentChar = ShiftForward(fp_Src);
            sv_DoesNotEqualsString += fp_CurrentChar;

            fp_ProgramTokens.emplace_back(sv_DoesNotEqualsString, TokenType::DoesNotEquals, fp_CurrentLineNumber);
            break; //just iterate as normal, and make sure the equals character isn't double counted
        }
        break;
        case '+':
        {
            string f_PlusString = "+";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator                
                f_PlusString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_PlusString, TokenType::PlusEqualsOperator, fp_CurrentLineNumber);
                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            fp_ProgramTokens.emplace_back(f_PlusString, TokenType::AdditionOperator, fp_CurrentLineNumber);
            break; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
        }
        break;
        case '*':
        {
            string f_MultString = "*";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator                
                f_MultString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_MultString, TokenType::MultEqualsOperator, fp_CurrentLineNumber);
                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            fp_ProgramTokens.emplace_back(f_MultString, TokenType::MultiplicationOperator, fp_CurrentLineNumber);
            break; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow  
        }
        break;
        case '-': //negative numbers are parsed as negative unary operator behind a number
        {
            string f_TypeArrow = "-"; //ik whats coming next, but for conventions sake

            while (fp_Src.size() > 0 and Peek(fp_Src) == '-')
            {
                fp_CurrentChar = ShiftForward(fp_Src);
                f_TypeArrow += fp_CurrentChar;
            }

            if (Peek(fp_Src) != '>' and Peek(fp_Src) != '=') //fuck it we ball, we deal with minus here BROTHERS
            {
                fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::NegativeOperator, fp_CurrentLineNumber);
                break; //start loop again or hit error u choose owo
            }

            fp_CurrentChar = ShiftForward(fp_Src); //look for equals or arrow tip uwu

            if (fp_CurrentChar == '>') //this goes first so that subtraction doesn't get confused with '->', since they both have only 1 dash
            {
                f_TypeArrow += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::TypeArrow, fp_CurrentLineNumber);
                break; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (fp_CurrentChar == '=' and f_TypeArrow.size() == 1) //check for single minus equals so '-=' not '-----=' srry m8 thats 2 far for m9
            {
                f_TypeArrow += fp_CurrentChar; //IDC THAT ITS NOT A TYPEARROW
                fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::MinusEqualsOperator, fp_CurrentLineNumber);
                break; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (fp_CurrentChar == '=' and f_TypeArrow.size() > 1) //check for single minus equals so '-=' not '-----=' srry m8 thats 2 far for m9
            {
                logger->Error(format("Error at Line Number: {}, minus equals definition has too many dashes COMON", fp_CurrentLineNumber), "Lexer");
                fp_Src.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                return false;
            }
            else //if we have more than one consecutive '-', then it's a mistake regardless of what you were trying to do // Handle error: Unterminated type arrow
            {
                logger->Error(format("Error at Line Number: {}", fp_CurrentLineNumber), "Lexer");
                logger->Warning("Unterminated type arrow brother!, looks like you're missing an arrow head to your type arrow definition", "Lexer");
                fp_Src.clear(); //dump the source code vector, so that the compiler will stop processing the source code
                return false;
            }
        }
        break;
        case '/':
        {
            string f_DivString = "/";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator
                f_DivString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_DivString, TokenType::DivEqualsOperator, fp_CurrentLineNumber);

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (Peek(fp_Src) == '/')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //get on '/' character then start parsing comment
                fp_IsCurrentlyInsideComment = true;
                break;
            }

            fp_ProgramTokens.emplace_back(f_DivString, TokenType::DivisionOperator, fp_CurrentLineNumber);
        }
        break;
        case '%':
        {
            string f_ModString = "%";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator                
                f_ModString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_ModString, TokenType::ModuloEqualsOperator, fp_CurrentLineNumber);
                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            fp_ProgramTokens.emplace_back(f_ModString, TokenType::ModulusOperator, fp_CurrentLineNumber);
            break; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow
        }
        break;
        case '<': //idrc about inserting this into the branch above with '>'
        {
            string f_LesserThanString = "<";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src);
                f_LesserThanString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::LesserThanOrEqual, fp_CurrentLineNumber);
                break; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (Peek(fp_Src) == '<')
            {
                fp_CurrentChar = ShiftForward(fp_Src);
                f_LesserThanString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::BitshiftLeftOperator, fp_CurrentLineNumber);

                if (Peek(fp_Src) == '=')
                {
                    fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator
                    f_LesserThanString += fp_CurrentChar;
                    fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::BitshiftLeftEquals, fp_CurrentLineNumber);

                    break; //just iterate as normal, and make sure the equals character isn't double counted
                }

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            //handles the case for just '<'
            fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::LesserThan, fp_CurrentLineNumber);
            break;
        }
        break;
        case '>':
        {
            string f_GreaterThanString = ">";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src);
                f_GreaterThanString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::GreaterThanOrEqual, fp_CurrentLineNumber);

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (Peek(fp_Src) == '>')
            {
                fp_CurrentChar = ShiftForward(fp_Src);
                f_GreaterThanString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::BitshiftRightOperator, fp_CurrentLineNumber);

                if (Peek(fp_Src) == '=')
                {
                    fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator
                    f_GreaterThanString += fp_CurrentChar;
                    fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::BitshiftRightEquals, fp_CurrentLineNumber);

                    break; //just iterate as normal, and make sure the equals character isn't double counted
                }

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            //handles the case for just '>'
            fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::GreaterThan, fp_CurrentLineNumber);
        }
        break;
        case '&':
        {
            string sv_And = "&";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator
                sv_And += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(sv_And, TokenType::BitAndEquals, fp_CurrentLineNumber);

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            fp_ProgramTokens.emplace_back(sv_And, TokenType::Ampersand, fp_CurrentLineNumber);
        }
        break;
        case '|':
        {
            string sv_BitOr = "|";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator
                sv_BitOr += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(sv_BitOr, TokenType::BitOrEquals, fp_CurrentLineNumber);

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            fp_ProgramTokens.emplace_back(sv_BitOr, TokenType::BitOrOperator, fp_CurrentLineNumber);
        }
        break;
        case '~':
            fp_ProgramTokens.emplace_back(fp_CurrentChar, TokenType::BitNotOperator, fp_CurrentLineNumber);
            break;
        case '^':
        {
            string sv_BitXor = "^";

            if (Peek(fp_Src) == '=')
            {
                fp_CurrentChar = ShiftForward(fp_Src); //look for an equals sign for the "+=" operator
                sv_BitXor += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(sv_BitXor, TokenType::BitXorEquals, fp_CurrentLineNumber);

                break; //just iterate as normal, and make sure the equals character isn't double counted
            }

            fp_ProgramTokens.emplace_back(sv_BitXor, TokenType::BitXorOperator, fp_CurrentLineNumber);
        }
        break;
        default:
            return false;
        }

        return true; //otherwise the continue will just move the current over-stepped character back to the top of the lexer's logical flow   
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
                Token sv_NumberToken = LexNumber(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, logger);

                if (sv_NumberToken.m_Type == TokenType::NO_TOKEN_VALUE)
                {
                    logger->Error(format("Error at Line Number: {}, failed to Lex number definition", f_CurrentLineNumber), "Lexer");
                    return false;
                }

                fp_Tokens.push_back(sv_NumberToken);

                continue; //move to next iteration
            }
            
            //////////////////////////////////////////////////////////// Formatted Strings ////////////////////////////////////////////////////////////

            else if (f_CurrentChar == 'f' and Peek(fp_SourceCode) == '"')
            {
                f_CurrentChar = ShiftForward(fp_SourceCode); //-->'"' 

                string f_FormattedString; //default val at ""

                f_CurrentChar = ShiftForward(fp_SourceCode); //shift twice since we wanna enter the string quotes

                if (f_CurrentChar == '"') //push back an empty string ig idk
                {
                    fp_Tokens.emplace_back("", TokenType::StringLiteral, f_CurrentLineNumber); //need this for the base case recursion of parse reg expr
                    continue;
                }

                f_FormattedString += f_CurrentChar; //add letter knowing it isnt just f"" anymore uwu

                bool f_IsEscapeCharacter = false;
                bool f_IsStringStart = true;

                //////////////////////////////////////////////////////////// Main Loop ////////////////////////////////////////////////////////////

                while (fp_SourceCode.size() > 0 and f_CurrentChar != '"')
                {
                    f_CurrentChar = ShiftForward(fp_SourceCode);

                    //////////////////////////////////////////////////////////// Handle Escape Characters ////////////////////////////////////////////////////////////

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
                            break; //shifts forward at the bottom of loop
                        }
                    }
                    //////////////////////////////////////////////////////////// String Inserts ////////////////////////////////////////////////////////////
                    else if (f_CurrentChar == '{')
                    {
                        if (f_IsStringStart) //used for properly insterting the string start, every intermediate string is just a plain StringLiteral, so the parser can do smth like: while(fp_CurrentToken.m_Type != FormattedStringLiteralEnd){ ... }
                        {
                            fp_Tokens.emplace_back(f_FormattedString, TokenType::FormattedStringLiteralStart, f_CurrentLineNumber);
                            f_IsStringStart = false; //set to false now since we hit the beginning uwu
                        }
                        else
                        {
                            fp_Tokens.emplace_back(f_FormattedString, TokenType::StringLiteral, f_CurrentLineNumber);
                        }

                        //needs to go after string literal uwu
                        fp_Tokens.emplace_back("{", TokenType::OpenBracket, f_CurrentLineNumber); //need this for the base case recursion of parse reg expr

                        f_FormattedString = ""; //reset string value for main while-loop, this if-statement should terminate within one main-while loop so intermediate, or end strings can be found to support : f"hey {name} i like {interest} uwu!"
                        //used for tracking identifiers, so I can do this iteratively instead of definish a recursive LexNumber style function

                        /*
                        hard to use a LexString function for both spots is hard since the functions are just barely different, where the end condition for lexing a normal string is '"', a FormattedStringStart ends when it finds '{',

                        if no insert values are found then the Token gets lexed as a singular FormattedStringEnd, so when the parser sees that it just assumes the programmer typed smth like : print(f"Hello World"), so tha parser can
                        interpret that as a a regular StringLiteral
                        */

                        //////////////////////////////////////////////////////////// Parse Inserted String Value ////////////////////////////////////////////////////////////
                        while (fp_SourceCode.size() > 0 and f_CurrentChar != '}')
                        {
                            f_CurrentChar = ShiftForward(fp_SourceCode);

                            if (isspace(f_CurrentChar))
                            {
                                continue;
                            }

                            if (isdigit(f_CurrentChar))
                            {
                                Token sv_NumberToken = LexNumber(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, logger);

                                if (sv_NumberToken.m_Type == TokenType::NO_TOKEN_VALUE)
                                {
                                    logger->Error(format("Error at Line Number: {}, failed to parse number inside formatted string", f_CurrentLineNumber), "Lexer");
                                    return false;
                                }

                                fp_Tokens.push_back(sv_NumberToken);
                                continue; //move to next iteration to shift forwards uwu
                            }

                            if (isalpha(f_CurrentChar))
                            {
                                fp_Tokens.emplace_back
                                (
                                    LexWord(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber), //dont shift off current character since LexWord consumes the entry token
                                    TokenType::UserIdentifier,
                                    f_CurrentLineNumber
                                );

                                continue; //LexWord ends f_CurrentChar on the last alpha character in sequence uwu, could look for primitive types or other keywords but dont really care here atm
                            }

                            if (LexOperator(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, f_IsCurrentlyInsideComment, logger))
                            {

                                continue;
                            }

                            if (f_IsCurrentlyInsideComment) //THROW ERROR: comment not allowed here uwu
                            {

                                return false;
                            }

                            switch (f_CurrentChar)
                            {
                            case '"':  //THROW ERROR: string terminated before formatted variable was closed
                            {
                                logger->Error(format("Unterminated variable inside your string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                                fp_SourceCode.clear();
                                return false;
                            }
                            break;
                            case '.': //assuming identifier since we check for number above
                            {
                                fp_Tokens.emplace_back(".", TokenType::Dot, f_CurrentLineNumber); //don't add current char to string insert since the last added later is the end of the identifier
                            }
                            break;
                            case '(': //assuming this is only used for 
                            {
                                fp_Tokens.emplace_back("(", TokenType::OpenParen, f_CurrentLineNumber); //don't add current char to string insert since the last added later is the end of the identifier
                            }
                            break;
                            case ')': //process bracket uwu
                            {
                                fp_Tokens.emplace_back(")", TokenType::CloseParen, f_CurrentLineNumber); //don't add current char to string insert since the last added later is the end of the identifier
                            }
                            break;
                            case '}': //needed for parsing uwu
                                fp_Tokens.emplace_back("}", TokenType::CloseBracket, f_CurrentLineNumber); //need this for the base case recursion of parse reg expr
                            break;
                            case '\0': //found end of file uwu
                            {

                            }
                            break;
                            default:
                                break;
                            }
                        }

                        if (f_CurrentChar != '}') //THROW ERROR: string terminated before formatted variable was closed , used to catch EOF stuff
                        {
                            logger->Error(format("Unterminated variable inside your string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                            fp_SourceCode.clear();
                            return false;
                        }

                        //no shift here since --> '}', and top of loop will shift off of it
                    }
                    else
                    {
                        f_FormattedString += f_CurrentChar; //proceed as usual
                    }
                }

                // Check if we've ended on the closing quotation mark
                if (f_CurrentChar != '"')  // Handle error: Unterminated string literal, and exit program execution
                {
                    logger->Error(format("Unterminated string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                    fp_SourceCode.clear();
                    return false;
                }

                // Push the final string token without the quotes
                fp_Tokens.emplace_back(f_FormattedString, TokenType::FormattedStringLiteralEnd, f_CurrentLineNumber);
                //move to next iteration, we shift here because f_CurrentChar is pointing -> ' " ' 
                continue; //moves to main lexing while-loop
            }

            //////////////////////////////////////////////////////////// Alphabetic Characters (outside string) ////////////////////////////////////////////////////////////

            else if (isalpha(f_CurrentChar) or f_CurrentChar == '_') //used for keywords, and user identifiers like enum, class or var names
            {
                string f_Identifier = LexWord(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber);

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

            if (LexOperator(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, f_IsCurrentlyInsideComment, logger)) //returns true for either lexed
            {
                continue;
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