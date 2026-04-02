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
#include "Lexer.h"

namespace BongoJam {

    //////////////////////////////////////////////
    // Utility Functions
    //////////////////////////////////////////////

    bool
        LexNumber
        (
            VectorStream<char>& fp_Src,
            char fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber,
            Logger* logger
        )
    {
        string f_Number = ""; // >w< //get current char since its a digit

        f_Number += fp_CurrentChar; 

        if (not fp_Src.Peek(fp_CurrentChar))
        {
            logger->Error(format("Found END__OF__FILE while parsing a number! Why is there a number at the end of the file >O<? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
            return false;
        }

        while (isdigit(fp_CurrentChar))
        {
            fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
            f_Number += fp_CurrentChar;

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing a number! Why is '{}' at the end of the file >O<? Error occured at line number: {}", f_Number, fp_CurrentLineNumber), "Lexer");
                return false;
            }
        }

        //peek oversteps so this is kosher >w<

        if (fp_CurrentChar == '.') //used for handling decimal numbers eg. "let x->float = 3.14;"
        {
            fp_Src.ShiftForwardUnsafe(fp_CurrentChar); //shift -> '.'
            f_Number += fp_CurrentChar; //add the decimal so we're at: "69. (rest to be parsed)" currently

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing a decimal number! Why is the EOF after '{}' >O<? Error occured at line number: {}", f_Number, fp_CurrentLineNumber), "Lexer");
                return false;
            }

            if (not isdigit(fp_CurrentChar)) // Handle error: incomplete float definition -> '59. '
            {
                logger->Error(format("Error at Line Number: {}, unexpected symbol: '{}' found following a '.' brother!, looks like you've input a non-numeric symbol while defining a decimal number", fp_CurrentLineNumber, fp_CurrentChar), "Lexer");
                return false;
            }
            //is a digit so check for digit again to verify and shiftforwards for value uwu
            while (isdigit(fp_CurrentChar))
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
                f_Number += fp_CurrentChar;

                if (not fp_Src.Peek(fp_CurrentChar))
                {
                    logger->Error(format("Found END__OF__FILE while parsing a decimal number! Why is '{}' at the end of the file >O<? Error occured at line number: {}", f_Number, fp_CurrentLineNumber), "Lexer");
                    return false;
                }
            }

            //push a float
            fp_ProgramTokens.emplace_back(f_Number, TokenType::FloatNumber, fp_CurrentLineNumber); //No need for a continue here since the current character isnt a digit
        }

        //push an int by default because otherwise the float would return uwu
        fp_ProgramTokens.emplace_back(f_Number, TokenType::IntNumber, fp_CurrentLineNumber); //No need for a continue here since the current character isnt a digit

        return true;
    }

    bool //assumed being called within if(current char is alpha) { LexWord(...)}, only returns false if found EOF instead of a word uwu
        LexWord
        (
            string& fp_StringContainer, //mutable used to fill in return string uwu
            VectorStream<char>& fp_Src,
            char fp_CurrentChar,
            size_t& fp_CurrentLineNumber,
            Logger* logger
        )
    {
        fp_StringContainer += fp_CurrentChar; //add current character since its already on one

        if (not fp_Src.Peek(fp_CurrentChar))
        {
            logger->Error(format("Found END__OF__FILE while parsing '='! Did you forget to finish your does not equals or strictly equals statement >O<? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
            return false;
        }

        while ((isalpha(fp_CurrentChar) or isdigit(fp_CurrentChar) or fp_CurrentChar == '_'))//used for tracking identifier like idk PeachNode2D uwu
        {
            fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
            fp_StringContainer += fp_CurrentChar;

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing a word! What did you do to your program? this isn't a valid way to end a statement, Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return false;
            }
        }

        return true;
    } 

    int8_t
        LexOperator
        (
            VectorStream<char>& fp_Src,
            char fp_CurrentChar,
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

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing '='! Did you forget to finish your does not equals or strictly equals statement >O<? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            while (fp_CurrentChar == '=') //see if more equals, if there is shift and consume uwu
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
                f_EqualsString += '=';

                if (not fp_Src.Peek(fp_CurrentChar))
                {
                    logger->Error(format("Found END__OF__FILE while parsing '='! Did you forget to finish your does not equals or strictly equals statement >O<? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                    return LEX_OPERATOR_SYNTAX_ERROR;
                }
            }

            if (f_EqualsString.size() >= 2)
            {
                fp_ProgramTokens.emplace_back(f_EqualsString, TokenType::StrictlyEquals, fp_CurrentLineNumber);
            }
            else if (f_EqualsString.size() == 1)
            {
                fp_ProgramTokens.emplace_back(f_EqualsString, TokenType::Equals, fp_CurrentLineNumber);
            }

            return LEX_OPERATOR_FOUND_SOMETHING;
        }
        break;
        case '!': 
        {
            string sv_DoesNotEqualsString = "!";

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing '!'! Did you forget to finish your does not equals statement >O<? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }
            else if (fp_CurrentChar != '=') //handles the case for when nothing valid follows a '!' in the source code //THROW ERROR
            {
                logger->Error(format("Error at Line Number: {}, invalid token found, you used a '!' but maybe forgot a '=' after it", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }
            else
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
                sv_DoesNotEqualsString += '=';

                fp_ProgramTokens.emplace_back(sv_DoesNotEqualsString, TokenType::DoesNotEquals, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '+':
        {
            string f_PlusString = "+";

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing '+'! Did you forget to finish your addition/equals? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }
            else if (fp_CurrentChar == '=') //look for an equals sign for the "+=" operator 
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
                          
                f_PlusString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_PlusString, TokenType::PlusEqualsOperator, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING; 
            }
            else if (fp_CurrentChar == '+') //look for an pos sign for the "++" operator     
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                f_PlusString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_PlusString, TokenType::PlusPlusOperator, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING; 
            }
            else
            {
                fp_ProgramTokens.emplace_back(f_PlusString, TokenType::AdditionOperator, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '*':
        {
            string f_MultString = "*";

            if (not fp_Src.Peek(fp_CurrentChar))
            {
                logger->Error(format("Found END__OF__FILE while parsing '*'! Did you forget to finish your multiplication/equals statement >///<? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (fp_CurrentChar == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                f_MultString += '=';
                fp_ProgramTokens.emplace_back(f_MultString, TokenType::MultEqualsOperator, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING; 
            }
            else
            {
                fp_ProgramTokens.emplace_back(f_MultString, TokenType::MultiplicationOperator, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '-': //negative numbers are parsed as negative unary operator behind a number
        {
            string f_TypeArrow = "-"; //ik whats coming next, but for conventions sake

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '-'! Did you forget to finish your type arrow or subtraction statement uwu? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            while (sv_Peek == '-')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);
                f_TypeArrow += '-';

                if (not fp_Src.Peek(sv_Peek))
                {
                    logger->Error(format("Found END__OF__FILE while parsing '-'! Did you forget to finish your type arrow or subtraction statement uwu? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                    return LEX_OPERATOR_SYNTAX_ERROR;
                }
            }

            //peek will overstep the minus sign uwu but not move the position counter forward uwu

            if (sv_Peek != '>' and sv_Peek != '=') //fuck it we ball, we deal with minus here BROTHERS
            {
                if(f_TypeArrow.size() == 1) //IDC THAT ITS NOT A TYPEARROW
                {
                    fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::NegativeOperator, fp_CurrentLineNumber);
                    return LEX_OPERATOR_FOUND_SOMETHING; //start loop again or hit error u choose owo
                }
                else if (f_TypeArrow.size() == 2)
                {
                    fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::MinusMinusOperator, fp_CurrentLineNumber);
                    return LEX_OPERATOR_FOUND_SOMETHING; //start loop again or hit error u choose owo
                }
                else
                {
                    logger->Error(format("Error at Line Number: {}, too many '-' minus signs brother pick 1 for subtraction or 2 for the decrement operator uwu", fp_CurrentLineNumber), "Lexer");
                    return LEX_OPERATOR_SYNTAX_ERROR;
                }
            }

            fp_Src.ShiftForwardUnsafe(fp_CurrentChar); //can shift unsafe here since peek was valid uwu

            if (fp_CurrentChar == '>') //this goes first so that subtraction doesn't get confused with '->', since they both have only 1 dash
            {
                f_TypeArrow += '>';
                fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::TypeArrow, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (fp_CurrentChar == '=' and f_TypeArrow.size() == 1) //check for single minus equals so '-=' not '-----=' srry m8 thats 2 far for m9
            {
                f_TypeArrow += '='; 
                fp_ProgramTokens.emplace_back(f_TypeArrow, TokenType::MinusEqualsOperator, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (fp_CurrentChar == '=' and f_TypeArrow.size() > 1) //check for single minus equals so '-=' not '-----=' srry m8 thats 2 far for m9
            {
                logger->Error(format("Error at Line Number: {}, minus equals definition has too many dashes COMON", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }
            else //if we have more than one consecutive '-', then it's a mistake regardless of what you were trying to do // Handle error: Unterminated type arrow
            {
                logger->Error(format("Error at Line Number: {}", fp_CurrentLineNumber), "Lexer");
                logger->Warning("Unterminated type arrow brother!, looks like you're missing an arrow head to your type arrow definition", "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }
        }
        break;
        case '/':
        {
            string f_DivString = "/";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '/'! Did you forget to finish a division or comment statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar); 
                f_DivString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_DivString, TokenType::DivEqualsOperator, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING; //just iterate as normal, and make sure the equals character isn't double counted
            }
            else if (sv_Peek == '/')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar); //get on '/' character then start parsing comment
                fp_IsCurrentlyInsideComment = true;
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else if (sv_Peek == '*')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar); //get off '/' character then start parsing comment

                while (1)
                {
                    if (not fp_Src.ShiftForward(fp_CurrentChar) or not fp_Src.Peek(sv_Peek)) //--> alpha char hopefully uwu
                    {
                        logger->Error(format("Found END__OF__FILE! 7Unterminated comment block found UwU, Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                        return LEX_OPERATOR_SYNTAX_ERROR;
                    }

                    if (fp_CurrentChar == '*' and sv_Peek == '/')
                    {
                        fp_Src.ShiftForwardUnsafe(fp_CurrentChar); //move onto '/' so loop can shift onto new token uwu
                        break;
                    }
                    else if (fp_CurrentChar == '\n')
                    {
                        fp_CurrentLineNumber++;
                    }
                }

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else
            {
                fp_ProgramTokens.emplace_back(f_DivString, TokenType::DivisionOperator, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '%':
        {
            string f_ModString = "%";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '%'! Did you forget to finish a modulo statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar); //look for an equals sign for the "+=" operator                
                f_ModString += '=';
                fp_ProgramTokens.emplace_back(f_ModString, TokenType::ModuloEqualsOperator, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else
            {
                fp_ProgramTokens.emplace_back(f_ModString, TokenType::ModulusOperator, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '<': //idrc about inserting this into the branch above with '>'
        {
            string f_LesserThanString = "<";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '<'! Did you forget to finish a less than equals or bitshift/equals statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }
            else if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                f_LesserThanString += '=';
                fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::LesserThanOrEqual, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else if (sv_Peek == '<')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                f_LesserThanString += '<';

                if (not fp_Src.Peek(sv_Peek))
                {
                    logger->Error(format("Found END__OF__FILE while parsing '<'! Did you forget to finish your bitshift left operator? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                    return LEX_OPERATOR_SYNTAX_ERROR;
                }

                if (sv_Peek == '=')
                {
                    fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                    f_LesserThanString += '=';
                    fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::BitshiftLeftEquals, fp_CurrentLineNumber);

                    return LEX_OPERATOR_FOUND_SOMETHING;
                }
                else
                {
                    fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::BitshiftLeftOperator, fp_CurrentLineNumber);
                    return LEX_OPERATOR_FOUND_SOMETHING;
                }
            }
            else //handles the case for just '<'
            {
                fp_ProgramTokens.emplace_back(f_LesserThanString, TokenType::LesserThan, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '>':
        {
            string f_GreaterThanString = ">";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '>'! Did you forget to finish a greater than equals or bitshift/equals statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                f_GreaterThanString += fp_CurrentChar;
                fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::GreaterThanOrEqual, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else if (sv_Peek == '>')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                f_GreaterThanString += '>';

                if (not fp_Src.Peek(sv_Peek))
                {
                    logger->Error(format("Found END__OF__FILE while parsing '<'! Did you forget to finish your bitshift left operator? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                    return LEX_OPERATOR_SYNTAX_ERROR;
                }

                if (sv_Peek == '=')
                {
                    fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                    f_GreaterThanString += '=';
                    fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::BitshiftRightEquals, fp_CurrentLineNumber);

                    return LEX_OPERATOR_FOUND_SOMETHING;
                }
                else
                {
                    fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::BitshiftRightOperator, fp_CurrentLineNumber);
                    return LEX_OPERATOR_FOUND_SOMETHING;
                }
            }

            //handles the case for just '>'
            fp_ProgramTokens.emplace_back(f_GreaterThanString, TokenType::GreaterThan, fp_CurrentLineNumber);
            return LEX_OPERATOR_FOUND_SOMETHING;
        }
        break;
        case '&':
        {
            string sv_And = "&";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '&'! Did you forget to finish your bitand or bitand equals statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                sv_And += '=';
                fp_ProgramTokens.emplace_back(sv_And, TokenType::BitAndEquals, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else
            {
                fp_ProgramTokens.emplace_back(sv_And, TokenType::Ampersand, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '|':
        {
            string sv_BitOr = "|";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '|'! Did you forget to finish your bitor or (>w<) bitor equals statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                sv_BitOr += '=';
                fp_ProgramTokens.emplace_back(sv_BitOr, TokenType::BitOrEquals, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else
            {
                fp_ProgramTokens.emplace_back(sv_BitOr, TokenType::BitOrOperator, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        case '~':
            fp_ProgramTokens.emplace_back(fp_CurrentChar, TokenType::BitNotOperator, fp_CurrentLineNumber);
            return LEX_OPERATOR_FOUND_SOMETHING;
        case '^':
        {
            string sv_BitXor = "^";

            char sv_Peek;

            if (not fp_Src.Peek(sv_Peek))
            {
                logger->Error(format("Found END__OF__FILE while parsing '^'! Did you forget to finish your xor or (>w<) xor equals statement? Error occured at line number: {}", fp_CurrentLineNumber), "Lexer");
                return LEX_OPERATOR_SYNTAX_ERROR;
            }

            if (sv_Peek == '=')
            {
                fp_Src.ShiftForwardUnsafe(fp_CurrentChar);

                sv_BitXor += '=';
                fp_ProgramTokens.emplace_back(sv_BitXor, TokenType::BitXorEquals, fp_CurrentLineNumber);

                return LEX_OPERATOR_FOUND_SOMETHING;
            }
            else
            {
                fp_ProgramTokens.emplace_back(sv_BitXor, TokenType::BitXorOperator, fp_CurrentLineNumber);
                return LEX_OPERATOR_FOUND_SOMETHING;
            }
        }
        break;
        default:
            return LEX_OPERATOR_NOTHING_FOUND;
        }
    }
    
    //////////////////////////////////////////////
    // Tokenize Function
    //////////////////////////////////////////////

    bool
        Tokenize
        (
            VectorStream<char>&& fp_SourceCode,
            vector<Token>& fp_Tokens,
            Logger* logger
        )
    {
        size_t f_CurrentLineNumber = 1; //humans start at 1 cs reptiles start at 0 uwu

        char f_CurrentChar;

        bool f_IsCurrentlyInsideComment = false;

        while (1)
        {
            //////////////////// Iterate Current Character ////////////////////

            if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //idk only way i get nullptr is if vector iterator is at the end
            {
                break;
            }

            char f_PeekedChar;
            fp_SourceCode.Peek(f_PeekedChar);


            //////////////////// Handle Spaces, New-Lines, and Comments ////////////////////

            if (f_CurrentChar == '\n') //used to keep track of what line number we're at in the source code, we only have single line comments, so this is sufficient
            {
                f_IsCurrentlyInsideComment = false; //reset if in comment because checking if its true is more expensive then just setting it to false everytime uwu
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
                if (not LexNumber(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, logger))
                {
                    logger->Error(format("Error at Line Number: {}, failed to Lex number definition", f_CurrentLineNumber), "Lexer");
                    return false;
                }
                else
                {
                    continue; //move to next iteration
                }
            }
            
            //////////////////////////////////////////////////////////// Formatted Strings ////////////////////////////////////////////////////////////

            else if (f_CurrentChar == 'f' and (f_PeekedChar == '"' or f_PeekedChar == '@')) //only valid tokens after f if used for a formatted string, otherwise it'll fall down to the regular word/keyword checks uwu
            {
                fp_SourceCode.ShiftForwardUnsafe(f_CurrentChar); //-->'"' or '@', can do this since peek was successful

                if (f_CurrentChar == '@')
                {
                    if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //--> alpha char hopefully uwu
                    {
                        logger->Error(format("Found END__OF__FILE after a '@' brother! You seemed to pass a misformed formatted string uwu, Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                        return false;
                    } 

                    string f_ColourIdentifier;

                    if (not LexWord(f_ColourIdentifier, fp_SourceCode, f_CurrentChar, f_CurrentLineNumber, logger))
                    {
                        logger->Error(format("Found END__OF__FILE! You seemed to pass an incomplete coloured string uwu, Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
                        return false;
                    }

                    if (find(ANSI_COLOURS.begin(), ANSI_COLOURS.end(), f_ColourIdentifier) == ANSI_COLOURS.end()) //if identifier is not a keyword then its just tokenized assuming its a var name or smth
                    {
                        logger->Error(format("Error at Line Number: {}, expected a colour identifier but found: {} instead >:(", f_CurrentLineNumber, "@" + f_ColourIdentifier), "Lexer");
                        return false;
                    }
                    
                    fp_Tokens.emplace_back(f_ColourIdentifier, TokenType::Colourize, f_CurrentLineNumber);

                    if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //--> '"' hopefully uwu 
                    {
                        logger->Error(format("Found END__OF__FILE when opening '\"' was expected! You seemed to pass a misformed formatted string uwu, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                        return false;
                    }  

                    if (f_CurrentChar != '"') //WTF MAN
                    {
                        logger->Error(format("Error at Line Number: {}, expected text after colour identifier: '{}' but found: {} instead >:(", f_CurrentLineNumber, "@" + f_ColourIdentifier, f_CurrentChar), "Lexer");
                        return false;
                    }
                }

                string f_FormattedString; //default val at ""

                if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //shift twice since we wanna enter the string quotes
                {
                    logger->Error(format("Found END__OF__FILE when closing '\"' was expected! Unterminated formatted string found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                    return false;
                }   

                if (f_CurrentChar == '"') //push back an empty string ig idk
                {
                    fp_Tokens.emplace_back("", TokenType::StringLiteral, f_CurrentLineNumber); //need this for the base case recursion of parse reg expr
                    continue;
                }

                f_FormattedString += f_CurrentChar; //add letter knowing it isnt just f"" anymore uwu

                bool f_IsStringStart = true;

                //////////////////////////////////////////////////////////// Main Loop ////////////////////////////////////////////////////////////

                while (f_CurrentChar != '"')
                {
                    if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //shift twice since we wanna enter the string quotes
                    {
                        logger->Error(format("Found END__OF__FILE when closing '\"' was expected! Unterminated formatted string found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                        return false;
                    }

                    //////////////////////////////////////////////////////////// Keep Track of Line Number UwU ////////////////////////////////////////////////////////////

                    if (f_CurrentChar == '\n')
                    {
                        f_CurrentLineNumber++;
                        continue;
                    }

                    //////////////////////////////////////////////////////////// Handle Escape Characters ////////////////////////////////////////////////////////////

                    if (f_CurrentChar == '\\')
                    {
                        if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //shift to find specific escape character uwu
                        {
                            logger->Error(format("Found END__OF__FILE when closing escape character was expected! Unterminated formatted string found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                            return false;
                        }

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
                        hard to use a LexString function for both spots is hard since the functions are just barely different, where the end condition for lexing a normal string is '"', a FormattedStringStart ends when it finds '}',

                        if no insert values are found then the Token gets lexed as a singular FormattedStringEnd, so when the parser sees that it just assumes the programmer typed smth like : print(f"Hello World"), so tha parser can
                        interpret that as a a regular StringLiteral
                        */

                        //////////////////////////////////////////////////////////// Parse Inserted String Value ////////////////////////////////////////////////////////////
                        while (f_CurrentChar != '}')
                        {
                            if (not fp_SourceCode.ShiftForward(f_CurrentChar)) //shift twice since we wanna enter the string quotes
                            {
                                logger->Error(format("Found END__OF__FILE when closing '}}' was expected! Unterminated formatted string insert found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                                return false;
                            }

                            if (isspace(f_CurrentChar)) //spaces can be ignored here since it's not part of the literal string uwu
                            {
                                continue;
                            }

                            if (f_CurrentChar == '\n')
                            {
                                f_CurrentLineNumber++;
                                continue;
                            }

                            if (isdigit(f_CurrentChar))
                            {
                                if (not LexNumber(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, logger))
                                {
                                    logger->Error(format("Error at Line Number: {}, failed to parse number inside formatted string", f_CurrentLineNumber), "Lexer");
                                    return false;
                                }
                                else
                                {
                                    continue; //move to next iteration to shift forwards uwu
                                }
                            }

                            if (isalpha(f_CurrentChar))
                            {
                                string f_VariableInsert;

                                if (not LexWord(f_VariableInsert, fp_SourceCode, f_CurrentChar, f_CurrentLineNumber, logger)) //dont shift off current character since LexWord consumes the entry token
                                {
                                    logger->Error(format("Found END__OF__FILE when closing '}}' was expected! Unterminated formatted string insert found: '{}', Error occured at line number : {}", f_VariableInsert, f_CurrentLineNumber), "Lexer");
                                    return false;
                                }

                                fp_Tokens.emplace_back
                                (
                                    f_VariableInsert,
                                    TokenType::UserIdentifier,
                                    f_CurrentLineNumber
                                );

                                continue; //LexWord ends f_CurrentChar on the last alpha character in sequence uwu, could look for primitive types or other keywords but dont really care here atm
                            }

                            int8_t f_Result = LexOperator(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, f_IsCurrentlyInsideComment, logger);

                            if (f_Result == LEX_OPERATOR_FOUND_SOMETHING) //just doing this since i dont feel like writing the cases into the current switch uwu
                            {
                                continue;
                            }
                            else if (f_Result == LEX_OPERATOR_SYNTAX_ERROR)
                            {

                                return false;
                            }

                            if (f_IsCurrentlyInsideComment) //THROW ERROR: comment not allowed here uwu
                            {
                                logger->Error(format("Error at Line Number: {}, found comment inside formatted string!", f_CurrentLineNumber), "Lexer");
                                return false;
                            }

                            switch (f_CurrentChar)
                            {
                            case '"':  //THROW ERROR: string terminated before formatted variable was closed
                            {
                                logger->Error(format("Unterminated variable inside your string literal, brother! Error occured at line number: {}", f_CurrentLineNumber), "Lexer");
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
                            default:
                                logger->Error(format("Unexpected token found inside formatted string: '{}', Error occured at line number: {}", f_CurrentChar, f_CurrentLineNumber), "Lexer");
                                return false;
                            }
                        }
                        //no shift here since --> '}', and top of loop will shift off of it
                    }
                    else
                    {
                        f_FormattedString += f_CurrentChar; //proceed as usual
                    }
                }

                // Push the final string token without the quotes
                fp_Tokens.emplace_back(f_FormattedString, TokenType::FormattedStringLiteralEnd, f_CurrentLineNumber);
                //move to next iteration, we shift here because f_CurrentChar is pointing -> ' " ' 
                continue; //moves to main lexing while-loop
            }

            //////////////////////////////////////////////////////////// Alphabetic Characters (outside string) ////////////////////////////////////////////////////////////

            else if (isalpha(f_CurrentChar) or f_CurrentChar == '_') //used for keywords, and user identifiers like enum, class or var names
            {
                string f_Identifier; 

                if (not LexWord(f_Identifier, fp_SourceCode, f_CurrentChar, f_CurrentLineNumber, logger))
                {
                    logger->Error(format("Found END__OF__FILE when identifier was expected, token in question: '{}' Error occured at line number: {}", f_Identifier, f_CurrentLineNumber), "Lexer");
                    return false;
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

            int8_t f_Result = LexOperator(fp_SourceCode, f_CurrentChar, fp_Tokens, f_CurrentLineNumber, f_IsCurrentlyInsideComment, logger);

            if (f_Result == LEX_OPERATOR_FOUND_SOMETHING) //returns true for either lexed, will throw error in function w.e
            {
                continue;
            }
            else if (f_Result == LEX_OPERATOR_SYNTAX_ERROR)
            {

                return false;
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

                if (not fp_SourceCode.ShiftForward(f_CurrentChar)) // Shift to the next character to start capturing the string, not the opening quote
                {
                    logger->Error(format("Found END__OF__FILE when closing '\"' was expected! Unterminated string found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                    return false;
                } 

                if (f_CurrentChar == '"') //push back an empty string ig idk
                {
                    fp_Tokens.emplace_back("", TokenType::StringLiteral, f_CurrentLineNumber); //need this for the base case recursion of parse reg expr
                    continue; //back to the main loop uwu
                }

                //////////////////////////////////////////////////////////// Main Loop ////////////////////////////////////////////////////////////

                while (f_CurrentChar != '"')
                {
                    //////////////////////////////////////////////////////////// Keep Track of Line Number UwU ////////////////////////////////////////////////////////////

                    if (f_CurrentChar == '\n')
                    {
                        f_CurrentLineNumber++;
                        continue;
                    }

                    //////////////////////////////////////////////////////////// Handle Escape Characters ////////////////////////////////////////////////////////////

                    if (f_CurrentChar == '\\')
                    {
                        if (not fp_SourceCode.ShiftForward(f_CurrentChar)) // Find Escape character owo
                        {
                            logger->Error(format("Found END__OF__FILE when expected escape character was expected! Unterminated string found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                            return false;
                        }

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

                    if (not fp_SourceCode.ShiftForward(f_CurrentChar)) // Shift to the next character to start capturing the string, not the opening quote
                    {
                        logger->Error(format("Found END__OF__FILE when closing '\"' was expected! Unterminated string found, Error occured at line number : {}", f_CurrentLineNumber), "Lexer");
                        return false;
                    }
                }
                
                // Push the final string token without the quotes
                fp_Tokens.emplace_back(f_CurrentStringLiteral, TokenType::StringLiteral, f_CurrentLineNumber);

                //move to next iteration, we dont shift here because f_CurrentChar is pointing -> ' " ' 
            }
            break;
            case '$':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::DollarSign, f_CurrentLineNumber);
                break;
            case '?':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::QuestionMark, f_CurrentLineNumber);
                break;
            case '@': //used when doing smth like myString = @bmaOtherString because thats totally valid or myString = @bma"11" uwu >W<
            {
                if (isalpha(f_PeekedChar)) //can use this since it was never utilized because we only reach here if every other branch is skipped uwu owo >O<
                {
                    fp_SourceCode.ShiftForwardUnsafe(f_CurrentChar); //can do this since f_CurrentChar = '@' and if peek fails it'll just spit out @ again which will fail isalpha() and fall into the else statement uwu!

                    string f_ColourIdentifier;

                    if (not LexWord(f_ColourIdentifier, fp_SourceCode, f_CurrentChar, f_CurrentLineNumber, logger))
                    {
                        logger->Error(format("Found END__OF__FILE when colour identifier was expected, token in question: '{}' Error occured at line number: {}", f_ColourIdentifier, f_CurrentLineNumber), "Lexer");
                        return false;
                    }

                    if (find(ANSI_COLOURS.begin(), ANSI_COLOURS.end(), f_ColourIdentifier) == ANSI_COLOURS.end()) //if identifier is not a keyword then its just tokenized assuming its a var name or smth
                    {
                        logger->Error(format("Error at Line Number: {}, expected a colour identifier but found: {} instead >:(", f_CurrentLineNumber, "@" + f_ColourIdentifier), "Lexer");
                        return false;
                    }

                    fp_Tokens.emplace_back(f_ColourIdentifier, TokenType::Colourize, f_CurrentLineNumber);

                    continue;
                }
                else
                {
                    logger->Error(format("Error at Line Number: {}, expected a colour identifier but found: '{}' instead >:(", f_CurrentLineNumber, f_PeekedChar), "Lexer");
                    return false;
                }

                fp_Tokens.emplace_back(f_CurrentChar, TokenType::AtSign, f_CurrentLineNumber);
            }
                break;
            case '#':
                fp_Tokens.emplace_back(f_CurrentChar, TokenType::HashTag, f_CurrentLineNumber);
                break;
            default:
                logger->Error(format("Lexing Error: Unrecognized character found in source code at line: {}", f_CurrentLineNumber), "Lexer");
                return false;
            }

        }

        fp_Tokens.emplace_back("END__OF__FILE", TokenType::ENDF, f_CurrentLineNumber); //label the end of the file i guess for some reason

        return true;
    }
}