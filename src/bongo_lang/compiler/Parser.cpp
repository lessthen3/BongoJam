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
#include "Parser.h"

namespace BongoJam {

    //////////////////////////////////////////////
    // Utility Functions
    //////////////////////////////////////////////

    [[nodiscard]] static inline bool
        IsUnaryOperator(const Token& fp_Token)
        noexcept
    {
        return fp_Token.m_Type == TokenType::NegativeOperator or fp_Token.m_Type == TokenType::Not or fp_Token.m_Type == TokenType::BitNotOperator;
    }

    [[nodiscard]] static inline bool
        IsTypename(const Token& fp_Token)
        noexcept
    {
        return
            fp_Token.m_Type == TokenType::UserIdentifier or //used for user defined types, if this isn"t a type we"ll catch it as a runtime error
            fp_Token.m_Type == TokenType::Int or //actually refering to the type itself and not the literal value
            fp_Token.m_Type == TokenType::UnsignedInt or
            fp_Token.m_Type == TokenType::Float or
            fp_Token.m_Type == TokenType::Double or
            fp_Token.m_Type == TokenType::String or
            fp_Token.m_Type == TokenType::Char or
            fp_Token.m_Type == TokenType::Bool or
            fp_Token.m_Type == TokenType::Void //voiddddddd;
            ;
    }

    [[nodiscard]] static inline bool
        IsValue(const Token& fp_Token)
        noexcept
    {
        return
            fp_Token.m_Type == TokenType::IntNumber or
            fp_Token.m_Type == TokenType::UnsignedIntNumber or
            fp_Token.m_Type == TokenType::Bool or
            fp_Token.m_Type == TokenType::StringLiteral or
            fp_Token.m_Type == TokenType::CharLiteral or
            fp_Token.m_Type == TokenType::FloatNumber or
            fp_Token.m_Type == TokenType::DoubleNumber or
            fp_Token.m_Type == TokenType::UserIdentifier //used for function calls and variables i guess idfk;
            ;
    }

    [[nodiscard]] static inline bool
        IsEOF(const Token& fp_Token)
        noexcept
    {
        return fp_Token.m_Type == TokenType::ENDF;
    }

    Parser::Parser(shared_ptr<Logger> fp_CompilerLogger)
    {
        if (fp_CompilerLogger)
        {
            parser_logger = fp_CompilerLogger;
        }
        else
        {
            PrintError("Tried to pass nullptr reference to Parser Instance >:^(");
        }
    }

    //////////////////////////////////////////////
    // Parsing Functions
    //////////////////////////////////////////////
    //Important: all parse functions call ShiftForward() first thing because the functions are assuming the context they are being called in.
    //that context being: you"ve found the corresponding indicator token, and the "stack pointer" aka f_CurrentToken is still sitting on that indicator token
    //eg. im parsing and find a "class" token, when I call ParseClassDeclaration(), the f_CurrentToken i pass is f_CurrentToken = "class" token.

    //this method will find the smallest possible expression formed by the tokens following the number token, and return it

    //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required

//================================================================================================= Numbers =================================================================================================//

    unique_ptr<Expr>
        Parser::ParseNumber
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        fp_ProgramTokens.Peek(fp_CurrentToken);

        //returning on close paren should kick start the paren chain back up to the original openparen() call
        switch (fp_CurrentToken.m_Type) //base cases that stops the recursive descent
        {
        case TokenType::SemiDot:
        case TokenType::CloseParen:
        case TokenType::Comma:
        case TokenType::CloseBracket:
        case TokenType::CloseSquareBracket:
            return make_unique<SingleValueExpr>(f_EntryToken); //entry token is the value, situation is . . . "string"; we're pointing at ';' rn so take the stashed entry token as val
        }

        if (not fp_ProgramTokens.ShiftForward(fp_CurrentToken)) //Shift forwards to look for a semi dot or more ops
        {
            parser_logger->Error(format("Found END__OF__FILE while parsing a number! Why is there a number at the end of the file >O<? Error occured at line number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Lexer");
            return nullptr;
        }

        switch (fp_CurrentToken.m_Type)
        {
        case TokenType::StrictlyEquals:
        case TokenType::As: //used for casting class instances, eg. myFunc( myClass as otherClass); (we call this func in parseFunc)
        case TokenType::Or: //idk y logical comps would be used here but y not UWUWUWUWUWUWU ----- MAYBE LEAVE THIS OUT
        case TokenType::And:
        case TokenType::BitshiftRightOperator:
        case TokenType::BitshiftLeftOperator:
        case TokenType::BitXorOperator:
        case TokenType::BitOrOperator:
        case TokenType::Ampersand: //BitAnd
        case TokenType::ModulusOperator:
        case TokenType::DivisionOperator:
        case TokenType::NegativeOperator:
        case TokenType::MultiplicationOperator:
        case TokenType::AdditionOperator:
        {
            Token f_OperatorToken = fp_CurrentToken;

            auto f_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_RegExpr)
            {
                parser_logger->Error(format("error parsing numerical expression at line: '{}', expected a value but found something very different! COME ON BROTHER!", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            //success! parsed expr properly and signing + sealing for delivery back up the call stack
            return make_unique<BinaryOperationExpr>
            (
                f_OperatorToken,
                make_unique<SingleValueExpr>(f_EntryToken),
                std::move(f_RegExpr)
            );
        }
        break;
        default:
            parser_logger->Error(format("found : '{}', when mathematical binary operation was expected", fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }
    }

    //================================================================================================= String Literals =================================================================================================//

    unique_ptr<Expr>
        Parser::ParseString //assuming the current token == StringLiteral
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            Token fp_Decorator
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        fp_ProgramTokens.Peek(fp_CurrentToken);

        //returning on close paren should kick start the paren chain back up to the original openparen() call
        switch (fp_CurrentToken.m_Type) //base cases that stops the recursive descent
        {
        case TokenType::SemiDot:
        case TokenType::CloseParen:
        case TokenType::Comma:
        case TokenType::CloseBracket:
        case TokenType::CloseSquareBracket:
            return make_unique<SingleValueExpr>(f_EntryToken, fp_Decorator); //entry token is the value, situation is . . . "string"; we're pointing at ';' rn so take the stashed entry token as val
        }

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //Shift confidently to look for an operator

        //fall through to switch now since the expr is assumed to be  ... "text") ... <-- probably an op after the closed bracket uwu

        switch (fp_CurrentToken.m_Type)
        {
        case TokenType::StrictlyEquals:
        case TokenType::AdditionOperator:
        {
            Token f_OperatorToken = fp_CurrentToken;

            auto f_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_RegExpr)
            {
                parser_logger->Error(format("error parsing numerical expression at line: '{}', expected a value but found something very different! COME ON BROTHER!", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            //success! parsed expr properly and signing + sealing for delivery back up the call stack
            return make_unique<BinaryOperationExpr>
                (
                    f_OperatorToken,
                    make_unique<SingleValueExpr>(f_EntryToken, fp_Decorator),
                    std::move(f_RegExpr)
                );
        }
        break;
        case TokenType::ModulusOperator:
        case TokenType::DivisionOperator:
        case TokenType::NegativeOperator:
        case TokenType::MultiplicationOperator:
        {
            parser_logger->Error(format("Math operation used improperly, tried to use operator: '{}' on a string at line: {}", fp_CurrentToken.m_Value, f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }
        break;
        default:
            parser_logger->Error(format("found : '{}', when processing : '{}' mathematical binary operation was expected", fp_CurrentToken.m_Value, f_EntryToken.m_Value), "Parser");
            return nullptr;
        }
    }

    //================================================================================================= User Identifiers =================================================================================================//

            //this function is used to deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();"		    handled by ParseNumber()
            //we also deal with expressions formed within method or function calls, so this method will return an expression ending with ';' or ',' eg. myFunc(1, 3 + otherFunc(), otherFunc() * 2);
            //																																					called					      called
    unique_ptr<Expr>
        Parser::ParseUserIdentifier //called when current token = user identifier, so it handles shifting
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            Token fp_Decorator
        )
    {
        Token f_NameToken = fp_CurrentToken; //used for tracking the user identifier used

        fp_ProgramTokens.Peek(fp_CurrentToken);

        //returning on close paren should kick start the paren chain back up to the original openparen() call
        switch (fp_CurrentToken.m_Type) //base cases that stops the recursive descent
        {
        case TokenType::SemiDot:
        case TokenType::CloseParen:
        case TokenType::Comma:
        case TokenType::CloseBracket:
        case TokenType::CloseSquareBracket:
            return make_unique<IdentifierExpr>(f_NameToken); //entry token is the value, situation is . . .["string"]; we're peeking at ']' rn so take the stashed entry token as val
        }

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward one token to check for any accessor symbols

        switch (fp_CurrentToken.m_Type)
        {
        case TokenType::OpenSquareBracket:
        {
            auto sv_IndexedContainerExpr = make_unique<ContainerIndexedAccessExpr>();

            sv_IndexedContainerExpr->ContainerName = f_NameToken;

            auto sv_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_RegExpr) //this will just instant return if it hits a end bracket and the expr will be a singlevalueexpr with type openbracket
            {
                parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when regular expression was expected inside list or dictionary item access expression", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }

            fp_ProgramTokens.ShiftForward(fp_CurrentToken);

            if (fp_CurrentToken.m_Type != TokenType::CloseSquareBracket) //reg expr ends on last part of expr so should be able to shift here
            {
                parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when ']' was expected to close list or dictionary item access expression", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }

            //now this is kinda weird, so for expressions like myList[0] = 2; the parser would just give up at ']' and call it a day
            //so solution, we can just call ParseUserIdentifier again and it will either return while staying on the ']' aka last parsed token of the expr as expected
            //if the next token is part of the base case so essentially it will leave the list access expression as is and return through the recursion
            //otherwise it will parse and find the binary or unary operators or hit EOF and return an error uwu
            //oh and this should also handle a chained call like myList[0].myVar = 2;

            auto sv_PossibleExprExtension = ParseUserIdentifier(f_NameToken, fp_ProgramTokens); //this only needs to be called once since it will recursively descend down the chain

            switch (sv_PossibleExprExtension->m_Domain) //time to find the return type and adjust the expression since we ran a binary op per se assuming the name of the list now we gotta replace first
            {
            case SyntaxNodeType::BinaryOperationExpr:
            {
                auto sv_RecastedBinaryExpr = unique_dynamic_cast<BinaryOperationExpr>(std::move(sv_PossibleExprExtension));
                sv_RecastedBinaryExpr->First = std::move(sv_IndexedContainerExpr);
                return sv_RecastedBinaryExpr;
            }
            break;
            case SyntaxNodeType::UnaryOperatorExpr:
            {
                auto sv_RecastedUnaryExpr = unique_dynamic_cast<UnaryOperatorExpr>(std::move(sv_PossibleExprExtension));
                sv_RecastedUnaryExpr->Value = std::move(sv_IndexedContainerExpr);
                return sv_RecastedUnaryExpr;
            }
            break;
            case SyntaxNodeType::VariableReassignmentExpr:
            {
                auto sv_RecastedVarReassignExpr = unique_dynamic_cast<VariableReassignmentExpr>(std::move(sv_PossibleExprExtension));
                sv_RecastedVarReassignExpr->VariableName = std::move(sv_IndexedContainerExpr);
                return sv_RecastedVarReassignExpr;
            }
            break;
            case SyntaxNodeType::SingleValueExpr: //IMPORTANT: this val is only returned if the base case is hit or a chained call like "myList[69].MyMethod()", otherwise its improper grammar and the compiler will find that ig
            {
                return sv_IndexedContainerExpr;
            }
            break;
            case SyntaxNodeType::FunctionCallExpr: //all the same shit chained expr
            case SyntaxNodeType::ContainerIndexedAccessExpr:
            case SyntaxNodeType::IdentifierExpr:
            {
                sv_IndexedContainerExpr->ChainedExpr = std::move(sv_PossibleExprExtension); //simply just return the identifier expr since the recursive call above will always parse x.y.z.func().uwu[0] ...
                return sv_IndexedContainerExpr;
            }
            break;
            default:
#ifdef BONGO_DEBUG
                PrintError(to_string(static_cast<int>(sv_PossibleExprExtension->m_Domain)) + " SYNTAX NODE TYPE");
#endif
                parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when regular expression was expected while parsing a contained index access expression", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser::ParseUserIdentifier()");
                return nullptr;
            }
        }
        break;
        case TokenType::Dot: //used for class calling, eg. "myClassInstance.myMethod(func args);" or "myClassInstance.myField = 69;"
        {
            fp_ProgramTokens.Peek(fp_CurrentToken);

            //if we arent accessing a method or field after the ".", then idc wtf u typed, that shit is getting thrown out dawg
            if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when class method or variable name was expected ", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                parser_logger->Warning("Something bad happened while calling a class object! Make sure you're calling the proper method or variable name", "Parser");
                return nullptr;
            }

            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forwards onto user identifier

            auto sv_IdentifierExpr = make_unique<IdentifierExpr>();

            auto sv_ChainedExpr = ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens); //will cache name string at beginning and will stack names until an expression is formed, could be singlevalue or function call or var reassign

            if (not sv_ChainedExpr)
            {
                parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when (variable/func/struct/class) name was expected ", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }

            sv_IdentifierExpr->ChainedExpr = std::move(sv_ChainedExpr);

            return sv_IdentifierExpr; //will recurse until hits base case or any other valid case at some point it'll crash uwu,
        }
        break;
        case TokenType::BitXorEquals:
        case TokenType::BitOrEquals:
        case TokenType::BitAndEquals:
        case TokenType::Equals:
        case TokenType::ModuloEqualsOperator:
        case TokenType::PlusEqualsOperator:
        case TokenType::MinusEqualsOperator:
        case TokenType::MultEqualsOperator:
        case TokenType::DivEqualsOperator:
        {
            unique_ptr<VariableReassignmentExpr> sv_VarChange = make_unique<VariableReassignmentExpr>();

            sv_VarChange->VariableName = make_unique<SingleValueExpr>(f_NameToken);
            sv_VarChange->Operator = fp_CurrentToken;
            sv_VarChange->NewValue = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_VarChange)
            {
                parser_logger->Error(format("Error at Line Number: {}, failed to parse variable reassignment for {}", fp_CurrentToken.m_SourceCodeLineNumber, f_NameToken.m_Value), "Parser");
                return nullptr;
            }

            return sv_VarChange;
        }
        break;
        case TokenType::OpenParen: //function or constructor call
        {
            unique_ptr<FunctionCallExpr> sv_FuncCallExpr = ParseFunctionCall(f_NameToken, fp_CurrentToken, fp_ProgramTokens);

            if (not sv_FuncCallExpr)
            {
                parser_logger->Error(format("Error at Line: {} invalid call to {}() function", f_NameToken.m_SourceCodeLineNumber, f_NameToken.m_Value), "Parser");
                return nullptr;
            }

            //current token should be ')', if theres nothing afterwards it'll immediately return after Peek()'ing in the recursive call, otherwise it'll chain appropriately

            auto sv_PossibleExprExtension = ParseUserIdentifier(f_NameToken, fp_ProgramTokens);

            switch (sv_PossibleExprExtension->m_Domain) //time to find the return type and adjust the expression since we ran a binary op per se assuming the name of the list now we gotta replace first
            {
            case SyntaxNodeType::BinaryOperationExpr:
            {
                auto sv_RecastedBinaryExpr = unique_dynamic_cast<BinaryOperationExpr>(std::move(sv_PossibleExprExtension));
                sv_RecastedBinaryExpr->First = std::move(sv_FuncCallExpr);
                return sv_RecastedBinaryExpr;
            }
            break;
            case SyntaxNodeType::UnaryOperatorExpr:
            {
                auto sv_RecastedUnaryExpr = unique_dynamic_cast<UnaryOperatorExpr>(std::move(sv_PossibleExprExtension));
                sv_RecastedUnaryExpr->Value = std::move(sv_FuncCallExpr);
                return sv_RecastedUnaryExpr;
            }
            break;
            case SyntaxNodeType::VariableReassignmentExpr:
            {
                auto sv_RecastedVarReassignExpr = unique_dynamic_cast<VariableReassignmentExpr>(std::move(sv_PossibleExprExtension));
                sv_RecastedVarReassignExpr->VariableName = std::move(sv_FuncCallExpr);
                return sv_RecastedVarReassignExpr;
            }
            break;
            case SyntaxNodeType::FunctionCallExpr: //all the same shit chained expr
            case SyntaxNodeType::ContainerIndexedAccessExpr:
            case SyntaxNodeType::IdentifierExpr: //IMPORTANT: this val is only returned if the base case is hit or a chained call like "myList[69].MyMethod()", otherwise its improper grammar and the compiler will find that ig
            {
                sv_FuncCallExpr->ChainedIdentifier = std::move(sv_PossibleExprExtension); //daisy chain identifiers into a tree
                return sv_FuncCallExpr;
            }
            break;
            default:
                PrintError(to_string(static_cast<int>(sv_PossibleExprExtension->m_Domain)) + " SYNTAX NODE TYPE");
                parser_logger->Error(format("Error at Line Number: {}, Found : '{}', when regular expression was expected while parsing a function call expression", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser::ParseUserIdentifier()");
                return nullptr;
            }

            //return with tupled or non tupled arg
            return sv_FuncCallExpr; //Current token should be ',' (nested function call or field access), ';'--single statement "MyFunc();" or '}' if inside a in-place struct construction
        }
        break;
        case TokenType::StrictlyEquals:
        case TokenType::As: //used for casting class instances, eg. myFunc( myClass as otherClass); (we call this func in parseFunc)
        case TokenType::Or: //idk y logical comps would be used here but y not UWUWUWUWUWUWU ----- MAYBE LEAVE THIS OUT
        case TokenType::And:
        case TokenType::BitshiftRightOperator:
        case TokenType::BitshiftLeftOperator:
        case TokenType::BitXorOperator:
        case TokenType::BitOrOperator:
        case TokenType::Ampersand: //BitAnd
        case TokenType::ModulusOperator:
        case TokenType::AdditionOperator: //used for: "myFunc( otherFunc() + 3, otherFunc() + anotherFunc())" or "func myFunc(
        case TokenType::NegativeOperator:
        case TokenType::MultiplicationOperator:
        case TokenType::DivisionOperator:
        {
            Token f_OperatorToken = fp_CurrentToken;

            auto f_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_RegExpr)
            {
                parser_logger->Error(format("Error at Line Number: {}, failed to parse regular expression, expected a value but found something very different! COME ON BROTHER!", f_NameToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            //success! parsed expr properly and signing + sealing for delivery back up the call stack
            return make_unique<BinaryOperationExpr>
            (
                f_OperatorToken,
                make_unique<SingleValueExpr>(f_NameToken),
                std::move(f_RegExpr)
            );
        }
        break;
        case TokenType::MinusMinusOperator:
        case TokenType::PlusPlusOperator:
        {
            return make_unique<UnaryOperatorExpr>
            (
                f_NameToken,
                make_unique<IdentifierExpr>(fp_CurrentToken)
            );
        }
        break;
        default:
            parser_logger->Error(format("Error at Line Number: {}, failed to parse regular expression, expected a value but found something very different! COME ON BROTHER!", f_NameToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }
    }

    //================================================================================================= Formatted String Literals =================================================================================================//

    unique_ptr<FmtdStringExpr> //assuming being called on FormattedStringStart, idk ab FormattedStringEnd
        Parser::ParseFormattedString
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<FmtdStringExpr> f_FormattedString = make_unique<FmtdStringExpr>();

        f_FormattedString->FullString.emplace_back(make_unique<SingleValueExpr>(fp_CurrentToken)); //push back starting token since parse function is only called when sitting on the formattedstring token uwu            

        //dont shift here since if its a formatted string w no inserts it will hit the condition here

        while (fp_CurrentToken.m_Type != TokenType::FormattedStringLiteralEnd)
        {
            fp_ProgramTokens.ShiftForward(fp_CurrentToken);

            if (fp_CurrentToken.m_Type == TokenType::OpenBracket)
            {
                auto sv_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_RegExpr)
                {
                    parser_logger->Error(format("Error at Line Number: {}, expected regular expression but found invalid token: '{}'", f_EntryToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                    return nullptr;
                }

                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //parse reg expr should end on the last processed token for the expr

                if (fp_CurrentToken.m_Type != TokenType::CloseBracket)
                {
                    parser_logger->Error(format("Error at Line Number: {}, expected: '}}' but found: {} instead, formatted string insert variable doesn't terminate properly!", f_EntryToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                    parser_logger->Error("Tip: when using '{}' for inserting variables, expressions must evaluate to a value and cannot be a statement that ends with ';'", "Parser");
                    return nullptr;
                }

                f_FormattedString->FullString.push_back(std::move(sv_RegExpr)); //push back regular expr to insert into string uwu
                //will exit branch and shift at top of loop off of '}'
            }
            else if (fp_CurrentToken.m_Type == TokenType::StringLiteral or fp_CurrentToken.m_Type == TokenType::FormattedStringLiteralEnd)
            {
                f_FormattedString->FullString.emplace_back(make_unique<SingleValueExpr>(fp_CurrentToken));
                //will shift at top
            }
            else
            {
                parser_logger->Error(format("Error at Line Number: {}, poorly formed formatted string found, parsing failed at token: '{}'", f_EntryToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }
        }

        if (fp_CurrentToken.m_Type != TokenType::FormattedStringLiteralEnd) //THROW ERROR:
        {
            parser_logger->Error(format("Error at Line Number: {}, unterminated formatted string! found: '{}' instead ", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }

        //UNSURE: should parse commands end on the last parsed token or the

        return f_FormattedString;
    }

    //================================================================================================= Parenthesis =================================================================================================//

    unique_ptr<ParenExpr>
        Parser::ParseOpenParen //should always return as a full unit of "(" + ")"
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        unique_ptr<ParenExpr> f_ParenExpr = make_unique<ParenExpr>();

        f_ParenExpr->Inside = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

        if (not f_ParenExpr->Inside)
        {
            parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift off of end paren since reg expr ends on last token that was part of the expression and stops as a base case at ')' and others uwu

        if (fp_CurrentToken.m_Type != TokenType::CloseParen)
        {
            parser_logger->Error(format("Error at Line Number: {}, Unterminated parenthesis, expected ')' but found : '{}'", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }

        return f_ParenExpr;
    }

    //================================================================================================= Regular Expressions =================================================================================================//

            //used for parsing any expr, and types it accordingly------ the assumed entry is a caller that is optimistic about the result being an expression so as a result the current token is right before said expression uwu
    unique_ptr<Expr>
        Parser::ParseRegularExpr
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        Token f_EntryToken = fp_CurrentToken;

        switch (fp_CurrentToken.m_Type)
        {
        case TokenType::FormattedStringLiteralStart:
        case TokenType::FormattedStringLiteralEnd:
        {
            auto sv_FmtString = ParseFormattedString(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_FmtString)
            {
                parser_logger->Error(format("found : '{}', when formatted string was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            return sv_FmtString;
        }
        break;
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

            return sv_ParsedNumber;
        }
        break;
        case TokenType::BitNotOperator: //no binary operator should show up here only unary uwu 
        case TokenType::NegativeOperator: // this will show up since it can be interpreted as unary as well
        case TokenType::Not:
        {
            auto sv_UnaryExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_UnaryExpr)
            {
                parser_logger->Error(format("Error at line: {}, was unable to parse unary expression ;w;", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            return make_unique<UnaryOperatorExpr>(fp_CurrentToken, std::move(sv_UnaryExpr));
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

            return sv_ParsedString;
        }
        break;
        case TokenType::UserIdentifier: //XXX: used for function calls and variables
        {
            auto sv_ParsedUserIdentifier = ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_ParsedUserIdentifier)
            {
                parser_logger->Error(format("found : '{}', while parsing a class/struct/variable/func name when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            return sv_ParsedUserIdentifier;
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

            return sv_ParsedOpenParen;
        }
        break;
        case TokenType::True: //same shit uwu
        case TokenType::False:
        {
            return make_unique<SingleValueExpr>(fp_CurrentToken);
        } //end of scope 
        break;
        case TokenType::OpenBracket: //scope declarations shouldn't exist here so it's assumed to be a struct construction
        {
            unique_ptr<InPlaceStructConstruction> sv_StructConstruc = ParseStructConstruction(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_StructConstruc)
            {
                parser_logger->Error(format("Parsing Error at line: {}, invalid struct construction declaration found inside function", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            return sv_StructConstruc;
            //current token should be '}' should be safe to shift  
        }
        break;
        case TokenType::Colourize:
        {
            Token sv_ColourToken = fp_CurrentToken;

            fp_ProgramTokens.Peek(fp_CurrentToken);

            switch (fp_CurrentToken.m_Type)
            {
            case TokenType::StringLiteral:
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift --> StringLiteral since we used Peek() for the switch OwO oWo

                auto sv_StringExpr = ParseString(fp_CurrentToken, fp_ProgramTokens, sv_ColourToken);

                return sv_StringExpr;
            }
            break;
            case TokenType::UserIdentifier:
            {

            }
            break;
            case TokenType::FormattedStringLiteralStart:
            {

            }
            break;
            default:
                parser_logger->Error(format("found : '{}', when user identifier or string literal was expected after colourize expression at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }
        }
        break;
        default:
            parser_logger->Error(format("found : '{}', when regular expression was expected at line number: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }
        //XXX: probaly should keep parsing after finding an error for intellisense and to list ALL errors not just one at a time so multiple compile attempts arent required

        return nullptr;
    }

    //================================================================================================= General Statement Blocks =================================================================================================//

    unique_ptr<StatementBlock>
        Parser::ParseStatementBlock
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            bool fp_IsInLoop
        )
    {
        unique_ptr<StatementBlock> f_StatementBloc = make_unique<StatementBlock>(); //bloc cause so im so random >w< rawr *wags tail

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        if (fp_CurrentToken.m_Type != TokenType::OpenBracket) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
            parser_logger->Warning("Unrecognized symbol found when parsing statement block! Try taking a look at your '{ }' code-body wrappers", "Parser");
            return nullptr;
        }

        bool f_ShouldParse = true;

        //parse the else code body
        while (f_ShouldParse)
        {
            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //we can shift now since we"re sitting on "(" right now, and have handled it"s logic   
            Token f_EntryToken = fp_CurrentToken;

            switch (fp_CurrentToken.m_Type) //no particular order
            {
            case TokenType::SemiDot: //just ignore stray semidots since each parse function checks for those anyways, a statement ends with a semi dot so its part of their parsing territory
                continue;
            case TokenType::Return:
            {
                auto sv_ReturnStatement = ParseReturnStatement(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ReturnStatement)
                {
                    parser_logger->Error(format("Error at Line Number: {}, invalid return statement found when parsing", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_ReturnStatement));
            }
            break;
            case TokenType::Break:
            {
                if (not fp_IsInLoop) //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}, illegal use of break outside of a loop", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    parser_logger->Warning("Tip: You are not allowed to use a break statement outside of a loop brother!", "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.emplace_back(make_unique<BreakStatement>());
            }
            break;
            case TokenType::Continue:
            {
                if (not fp_IsInLoop) //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}, illegal use of continue outside of a loop", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    parser_logger->Warning("You are not allowed to use a continue statement outside of a loop brother!", "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.emplace_back(make_unique<ContinueStatement>());
            }
            break;
            case TokenType::If:
            {
                unique_ptr<IfDeclaration> sv_NestedIfStatement = ParseIfBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

                if (not sv_NestedIfStatement)
                {
                    parser_logger->Error(format("Error at Line Number: {}", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_NestedIfStatement));
            }
            break;
            case TokenType::CloseBracket:
                f_ShouldParse = false; //end parsing since end of block was reached uwu
                break;
                //deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();" specifically
            case TokenType::UserIdentifier: //handles variables, function, and class instance names being passed as a single argument
            {
                auto sv_UserDefinedAction = ParseUserIdentifier(fp_CurrentToken, fp_ProgramTokens); //used for var reassignment, class method access

                if (not sv_UserDefinedAction)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, bad grammer found involving a user identifier", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_UserDefinedAction));
                //current token should be ';' should be safe to shift
            } //end of scope
            break;
            case TokenType::Var:
            {
                auto sv_VariableDefinition = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_VariableDefinition)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid variable definition found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_VariableDefinition));
                //current token should be ';' should be safe to shift
            }
            break;
            case TokenType::While:
            {
                auto sv_WhileDec = ParseWhileBlock(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_WhileDec)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid variable definition found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_WhileDec));
            }
            break;
            case TokenType::For:
            {
                auto sv_ForDec = ParseForBlock(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ForDec)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid variable definition found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_ForDec));
            }
            break;
            case TokenType::Try:
            {
                auto sv_TryCatch = ParseTryCatchBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

                if (not sv_TryCatch)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid try-catch definition found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_TryCatch));
                //~~~~~~~~~ uwu
            }
            break;
            case TokenType::OpenParen:
            {
                auto sv_OpenParen = ParseOpenParen(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_OpenParen)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid expression found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_OpenParen));
            }
            break;
            case TokenType::Const:
            {
                f_EntryToken = fp_CurrentToken;

                auto sv_ConstStatement = ParseConstant(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ConstStatement)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid const definition found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_ConstStatement));
            }
            break;
            case TokenType::Static:
            {
                f_EntryToken = fp_CurrentToken;

                auto sv_StaticStatement = ParseStatic(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_StaticStatement)
                {
                    parser_logger->Error(format("Parsing Error at line:'{}', couldn't parse static whatever ", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StatementBloc->CodeBody.push_back(std::move(sv_StaticStatement)); //scoped static var idk should only be used in functions but whatever
            }
            break; //should shiftforward at top of loop at work fine uwu
            default:
                parser_logger->Error(format("Error at Line Number: {}, found : '{}', when statement was expected inside a code block", to_string(fp_CurrentToken.m_SourceCodeLineNumber), fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }
        }

        if (fp_CurrentToken.m_Type != TokenType::CloseBracket) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            parser_logger->Warning("Unrecognized expression found while declaring your function brother! Try taking a look at your function parameter(s) definition", "Parser");
            return nullptr;
        }

        return f_StatementBloc;
    }

    //================================================================================================= Try/Catch =================================================================================================//

    unique_ptr<TryCatchDeclaration>
        Parser::ParseTryCatchBlock //catch should come immediately after try always 
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            bool fp_IsInLoop
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<TryCatchDeclaration> f_TryCatchDec = make_unique<TryCatchDeclaration>();

        {
            auto sv_TryCodeBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

            if (not sv_TryCodeBlock)
            {
                parser_logger->Error(format("Error at Line Number: {}, unable to parse try code block >O<", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            f_TryCatchDec->TryBlock = std::move(sv_TryCodeBlock);
        }
        {
            auto sv_CatchCondition = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_CatchCondition)
            {
                parser_logger->Error(format("Error at Line Number: {}, unable to parse catch() condition, go read a book", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            f_TryCatchDec->CatchCondition = std::move(sv_CatchCondition);
        }
        //WARNING: NOT SURE IF I NEEDA SHIFT HERE OR NOT IK STATEMENTBLOCK ENDS ON A CLOSEBRACKET ALWAYS SO IDK WE SEE UWU
        {
            auto sv_CatchBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

            if (not sv_CatchBlock)
            {
                parser_logger->Error(format("Error at Line Number: {}, unable to parse catch code block ^_^", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                return nullptr;
            }

            f_TryCatchDec->TryBlock = std::move(sv_CatchBlock);
        }

        return f_TryCatchDec;
    }

    //================================================================================================= If/Else =================================================================================================//

    unique_ptr<IfDeclaration>
        Parser::ParseIfBlock
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            bool fp_IsInLoop,
            bool fp_IsInsideElseIf
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<IfDeclaration> f_IfDec = make_unique<IfDeclaration>();

        size_t f_IfLineNumber = fp_CurrentToken.m_SourceCodeLineNumber;

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        if (fp_CurrentToken.m_Type != TokenType::OpenParen) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}, expected '(' when parsing if-statement, try checking your if-statement condition", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            parser_logger->Warning("Something bad happened while declaring your if statement brother! Try looking at your brackets on the if-statement", "Parser");
            return nullptr;
        }

        //parse through the condition ' if "( cond )" '

        auto f_ConditionExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

        if (not f_ConditionExpr)
        {
            parser_logger->Error(format("Error at Line Number: {}, couldn't parse the condition for your if statement~~ nyah", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }
        //else if (f_ConditionExpr->EvaluatesTo != TokenType::Bool) //make sure condition evaluates to a proper condition uwu
        //{
        //    parser_logger->Error(format("Error occured at line: {}, if-statement condition doesn't evaluate to a bool, wtf man?", f_EntryToken.m_SourceCodeLineNumber), "Parser");
        //    return nullptr;
        //}

        f_IfDec->m_Condition = std::move(f_ConditionExpr);

        //needa shift forward because the last token should be the end of the expression still so if(..."text") <-- current token should = 'text' rn since the close paren shouldnt parse 

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        if (fp_CurrentToken.m_Type != TokenType::CloseParen)
        {
            parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        //fp_CurrentToken = ')' right now
        //assuming that when the bracket count goes to 0, we are outside of the if statement condition, we now check for the first curly-brace

        auto f_CodeBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

        if (not f_CodeBlock)
        {
            parser_logger->Error(format("Error at Line Number: {}, unable to parse if statement code block >w<", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        f_IfDec->CodeBody = std::move(f_CodeBlock->CodeBody);

        //end of processing code block, ParseStatementBlock exits on '}' so fp_CurrentToken = '}'

        if (not fp_IsInsideElseIf)
        {
            fp_ProgramTokens.Peek(fp_CurrentToken);

            //idk wtf i had here before but yeah keep looping while else if's are found uwu
            while (fp_CurrentToken.m_Type == TokenType::Elif)
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift onto elif token

                Token f_ElseIfEntryToken = fp_CurrentToken;

                auto sv_FallThroughCondition = ParseIfBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop, true);

                if (not sv_FallThroughCondition)
                {
                    parser_logger->Error(format("Error at Line Number: {}, unable to parse else-if statement >w<", f_ElseIfEntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_IfDec->m_ElseIfStatements.push_back(std::move(sv_FallThroughCondition));
            }

            fp_ProgramTokens.Peek(fp_CurrentToken);

            if (fp_CurrentToken.m_Type == TokenType::Else)
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift onto else token

                auto f_ElseStatement = ParseElseBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

                if (not f_ElseStatement)
                {
                    parser_logger->Error(format("Error while defining an else statement at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_IfDec->m_ElseStatement = std::move(f_ElseStatement);
            }
        }

        return f_IfDec;
    }

    unique_ptr<ElseDeclaration>
        Parser::ParseElseBlock
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            bool fp_IsInLoop
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<ElseDeclaration> f_ElseDec = make_unique<ElseDeclaration>();

        auto f_StatementBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, fp_IsInLoop);

        if (not f_StatementBlock)
        {
            parser_logger->Error(format("Error occured at line: '{}' while trying to parse your else statement >:^(", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        f_ElseDec->CodeBody = std::move(f_StatementBlock->CodeBody); //move codebody over idk kinda lazy but whatever fits the ptr return error structure better maybe ptr -> vec but idk eh

        return (f_ElseDec);
    }

    //================================================================================================= While Loop =================================================================================================//

    unique_ptr<WhileLoopDeclaration>
        Parser::ParseWhileBlock
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<WhileLoopDeclaration> f_WhileDec = make_unique<WhileLoopDeclaration>();

        auto f_ParsedCondition = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);
        if (not f_ParsedCondition)
        {
            parser_logger->Error(format("Error occured at line: {} while trying to parse your while statement Owo", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }
        else if (f_ParsedCondition->EvaluatesTo != TokenType::Bool) //make sure condition evaluates to a proper condition uwu
        {
            parser_logger->Error(format("Error occured at line: {}, while-loop condition doesn't evaluate ->bool, wtf man?", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }
        f_WhileDec->m_Condition = std::move(f_ParsedCondition);

        auto f_ParsedStatementBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, true);
        if (not f_ParsedStatementBlock)
        {
            parser_logger->Error(format("Error occured at line: {}, while-loop code block is bad and doesn't work try again but better this time", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        f_WhileDec->CodeBody = std::move(f_ParsedStatementBlock->CodeBody);

        return f_WhileDec;
    }

    //================================================================================================= For Loop =================================================================================================//

    unique_ptr<ForLoopDeclaration>
        Parser::ParseForBlock
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<ForLoopDeclaration> f_ForDec = make_unique<ForLoopDeclaration>();

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        if (fp_CurrentToken.m_Type != TokenType::OpenParen)
        {
            parser_logger->Error(format("Error occured at line: {}, expected '(' at for-loop condition, found: '{}' instead", f_EntryToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        //////////// parse for loop condition

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        if (fp_CurrentToken.m_Type != TokenType::CloseParen)
        {
            parser_logger->Error(format("Error occured at line: {}, expected ')' at for-loop condition, found: '{}' instead", f_EntryToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }

        auto f_ParsedStatementBlock = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, true);
        if (not f_ParsedStatementBlock)
        {
            parser_logger->Error(format("Error occured at line: {}, couldn't parse for-loop code block, u srs?", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        f_ForDec->CodeBody = std::move(f_ParsedStatementBlock->CodeBody);

        return f_ForDec;
    }

    //================================================================================================= Function Return =================================================================================================//

    unique_ptr<ReturnStatement>
        Parser::ParseReturnStatement
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
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

        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        if (fp_CurrentToken.m_Type != TokenType::SemiDot)
        {
            parser_logger->Error(format("Error at Line Number: {}, imporperly terminated return statement, did you forget a ';' at the end of your return statement?", f_EntryToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        return f_ReturnStatement;
    }

    //================================================================================================= Function Declaration =================================================================================================//

    unique_ptr<FuncDeclaration>
        Parser::ParseFuncDeclaration
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        unique_ptr<FuncDeclaration> f_FuncDec = make_unique<FuncDeclaration>();

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //assuming this is being called when fp_CurrentToken == TokenType::Func

        if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}, expected function name but found: '{}' instead >:^(", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            parser_logger->Warning("Something bad happened while declaring your function brother! Try taking a look at your function name definition", "Parser");
            return nullptr;
        }
        else if (STANDARD_FUNCTIONS.find(fp_CurrentToken.m_Value) != STANDARD_FUNCTIONS.end())
        {
            parser_logger->Error(format("Error at Line Number: {}, illegal function name: '{}()', PLEASE you are not allowed to declare a function that has the same name as a standard library function", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }

        f_FuncDec->m_FuncName = fp_CurrentToken; //record the function name
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an open paren

        if (fp_CurrentToken.m_Type != TokenType::OpenParen) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
            parser_logger->Warning("Something bad happened while declaring your function brother! Try taking a look at how you've placed your parenthesis", "Parser");
            return nullptr;
        }

        //if it"s an open paren then we"re clear to move forward and read the condition inside
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an open paren

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
                parser_logger->Error(format("Error at Line Number: {}, expected function name found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                parser_logger->Warning("Unrecognized symbol following an open parenthesis while declaring: " + f_FuncDec->m_FuncName.m_Value + "'s arguments brother! Try taking a look at your function argument(s) defintion", "Parser");
                return nullptr;
            }

            f_CurrentArgument.Name = fp_CurrentToken;
            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an open paren

            if (fp_CurrentToken.m_Type != TokenType::TypeArrow) //THROW ERROR
            {
                parser_logger->Error(format("Error at Line Number: {}, expected '->' but found: '{}' instead while defining function arguments for: '{}' ", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value, f_FuncDec->m_FuncName.m_Value), "Parser");
                parser_logger->Warning(format("Unrecognized symbol following a name definition while declaring function: {} arguments brother! Try taking a look at your type-arrows", f_FuncDec->m_FuncName.m_Value), "Parser");
                return nullptr;
            }

            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an open paren

            if (not IsTypename(fp_CurrentToken)) //THROW ERROR : invalid return type selected for function
            {
                parser_logger->Error
                (
                    format("Error at Line Number: {}, expected typename but found: '{}' instead while defining function arguments for: '{}' ", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value, f_FuncDec->m_FuncName.m_Value),
                    "Parser"
                );
                parser_logger->Warning
                (
                    format("Unrecognized var type found while declaring function: {} brother! Are you sure you've entered a valid type in {}'s argument definition?", f_FuncDec->m_FuncName.m_Value, f_FuncDec->m_FuncName.m_Value),
                    "Parser"
                );
                return nullptr;
            }

            f_CurrentArgument.Type = fp_CurrentToken; //type indices correspond to the func arg list
            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for default value or another argument

            //////////////////////////////////////////////////////////// Search for default value declaration ////////////////////////////////////////////////////////////
            if (fp_CurrentToken.m_Type == TokenType::Equals) //default values have to be literals, since its calculated at compile time, so no runtime values
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for literal default value

                if (not IsValue(fp_CurrentToken)) //THROW ERROR
                {
                    parser_logger->Error(format("Error at Line Number: {}, expected literal value inside function argument definition for {} but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value, fp_CurrentToken.m_Value), "Parser");
                    parser_logger->Warning(format("Tip: Most kinds of variables aren't allowed for function argument default values, only const static var's are allowed to be used"), "Parser");
                    //could add some debug functions here to query the found token, and make a guess at what was intended uwu, tahts for bongo or wait compilerdebugtools uwu
                    return nullptr;
                }

                //TODO: allow static, const compile time variables to be used for default values but not rn uwu
                //no need to parse an expr since it should always be a literal value no runtime bs for compile time functions
                f_CurrentArgument.DefaultValue = fp_CurrentToken; //type indices correspond to the func arg list 
            }

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
            else //THROW ERROR : general syntax error, unexpected symbol here means a symbol the compiler doesn't look for when parsing function arguments
            {
                Print("Token before error thrown is: " + fp_CurrentToken.m_Value + "at line: " + to_string(fp_CurrentToken.m_SourceCodeLineNumber));
                parser_logger->Error(format("Error at Line Number: {}, found a symbol that was not expected during function argument definition for {} and found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value, fp_CurrentToken.m_Value), "Parser");
                parser_logger->Warning(format("Tip: Try taking a look at your comma separation between function parameters", f_FuncDec->m_FuncName.m_Value), "Parser");
                return nullptr;
            }
        } //end of function arg parsing
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //if function definition was valid, then f_CurrentToken = ')' right now
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for a type arrow definition

        if (fp_CurrentToken.m_Type != TokenType::TypeArrow) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
            parser_logger->Warning("Unexpected symbol found in: " + f_FuncDec->m_FuncName.m_Value + "'s return type defintion! \n Try taking a look at your type-arrow definition", "Parser");
            return nullptr;
        }

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for a return type definition

        if (not IsTypename(fp_CurrentToken)) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            parser_logger->Warning("Invalid type found in: " + f_FuncDec->m_FuncName.m_Value + "'s return type defintion! \n Try taking a look at your type-arrow definition", "Parser");
            return nullptr;
        }

        f_FuncDec->m_FuncReturnType = fp_CurrentToken; //assign return type to the FuncDeclaration struct

        Token f_EntryToken = fp_CurrentToken;

        auto f_FuncBody = ParseStatementBlock(fp_CurrentToken, fp_ProgramTokens, false);

        if (not f_FuncBody)
        {
            parser_logger->Error(format("Error at Line Number: {}, unable to parse function : '{}' code body", f_EntryToken.m_SourceCodeLineNumber, f_FuncDec->m_FuncName.m_Value), "Parser");
            return nullptr;
        }

        f_FuncDec->CodeBody = std::move(f_FuncBody->CodeBody);

        //since we successfully parsed the function body, due to our while-loop condition, f_CurrentToken = '}',
        //we don't need to ShiftForward() here because when this function exits, it leaves that responsibility up to the while loop that called it
        //so fp_CurrentToken = '}' aka close bracket

        if (not ValidateFunctionReturnPaths()) //validate every path returns a value for even void funcs since the coder could've written a return inside a void function, gotta check for accidents
        {

            return nullptr;
        }

        return f_FuncDec;
    }

    //================================================================================================= Function Call =================================================================================================//

    unique_ptr<FunctionCallExpr> //Used for functions, standard functions and class constructors
        Parser::ParseFunctionCall
        (
            const Token fp_NameToken,
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        unique_ptr<FunctionCallExpr> f_FuncCallExpr = make_unique<FunctionCallExpr>();

        f_FuncCallExpr->FuncName = fp_NameToken; //record the function name

        fp_ProgramTokens.Peek(fp_CurrentToken);

        if (fp_CurrentToken.m_Type == TokenType::CloseParen) //check for a procedure ig
        {
            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift onto token
            return move(f_FuncCallExpr);
        }

        //FUNCTION ARGUMENTS
        while (not fp_ProgramTokens.IsEmpty())
        {
            Token f_EntryToken = fp_CurrentToken;

            auto f_FuncArg = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

            if (not f_FuncArg)
            {
                parser_logger->Error(format("Error at Line Number: {}, unable to parse {} function, parsing failed at token: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_FuncCallExpr->FuncName.m_Value, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }

            f_FuncCallExpr->Arguments.push_back(std::move(f_FuncArg)); //uwu

            fp_ProgramTokens.Peek(fp_CurrentToken);

            if (fp_CurrentToken.m_Type == TokenType::Comma)
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);
                continue;
            }
            else if (fp_CurrentToken.m_Type == TokenType::CloseParen)
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);
                break;
            }

            if (fp_CurrentToken.m_Type == TokenType::ENDF)
            {
                parser_logger->Error(format("Error at Line Number: {}, found END__OF__FILE when function argument was expected uwu", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
                return nullptr;
            }
        }

        if (fp_CurrentToken.m_Type != TokenType::CloseParen)
        {
            parser_logger->Error(format("Error at Line Number: {}, expected ')' after function call expression but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            return nullptr;
        }

        return f_FuncCallExpr;
    }

    //================================================================================================= Class Declaration =================================================================================================//
    
    unique_ptr<ClassDeclaration>
        Parser::ParseClassDeclaration
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens,
            bool fp_IsSingle
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<ClassDeclaration> f_ClassDec = make_unique<ClassDeclaration>();

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an user defined identifier eg. "class MyClass"

        if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)//if it"s a proper name defintion then we can now proceed forwards //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
            parser_logger->Warning("Something bad happened while declaring your class brother! Did you forget an open bracket?", "Parser");
            return nullptr;
        }

        f_ClassDec->ClassName = fp_CurrentToken;
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an open paren

        if (fp_CurrentToken.m_Type != TokenType::OpenBracket) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", to_string(fp_CurrentToken.m_SourceCodeLineNumber)), "Parser");
            parser_logger->Warning("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser");
            return nullptr;
        }

        bool f_IsParsing = true;
        uint8_t f_CurrentAccessLevel = 0;

        bool f_IsSingle = false; //used for nested single classes

        while (f_IsParsing) //parse main code block of class decl
        {
            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward since the current character is "{"
            f_EntryToken = fp_CurrentToken;

            switch (fp_CurrentToken.m_Type)
            {
            case TokenType::Var:
            {
                auto sv_FieldDec = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_FieldDec)
                {
                    parser_logger->Error(format("Error at Line Number: {}, unable to parse field declaration inside class : '{}'", fp_CurrentToken.m_SourceCodeLineNumber, f_ClassDec->ClassName.m_Value), "Parser");
                    return nullptr;
                }

                sv_FieldDec->Modifiers |= f_CurrentAccessLevel; //set access level

                f_ClassDec->Fields.push_back(std::move(sv_FieldDec));
            }
            break;
            case TokenType::Func:
            {
                Token f_EntryToken = fp_CurrentToken;

                auto sv_FuncDeclaration = ParseFuncDeclaration(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_FuncDeclaration)
                {
                    parser_logger->Error(format("Error at Line Number: {}, unable to parse func declaration inside class : '{}'", f_EntryToken.m_SourceCodeLineNumber, f_ClassDec->ClassName.m_Value), "Parser");
                    return nullptr;
                }

                sv_FuncDeclaration->Modifiers |= f_CurrentAccessLevel; //set access level

                if (sv_FuncDeclaration->m_FuncName.m_Value == f_ClassDec->ClassName.m_Value)
                {
                    f_ClassDec->Constructors.push_back(std::move(sv_FuncDeclaration));
                }
                else
                {
                    f_ClassDec->Methods.push_back(std::move(sv_FuncDeclaration));
                }
                //Success! function parsed uwu
            }
            break;
            case TokenType::Static:
            {
                auto sv_StaticDeclaration = ParseStatic(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_StaticDeclaration)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, couldn't parse static whatever ", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                switch (sv_StaticDeclaration->m_Domain)
                {
                case SyntaxNodeType::FuncDeclaration:
                {
                    auto sv_RecastedFuncDec = unique_dynamic_cast<FuncDeclaration>(std::move(sv_StaticDeclaration));
                    f_ClassDec->Methods.push_back(std::move(sv_RecastedFuncDec)); //global static var
                }
                break;
                case SyntaxNodeType::VarDeclaration:
                {
                    auto sv_RecastedVarDec = unique_dynamic_cast<VarDeclaration>(std::move(sv_StaticDeclaration));
                    f_ClassDec->Fields.push_back(std::move(sv_RecastedVarDec)); //global static var
                }
                break;
                default:
                    parser_logger->Error(format("Error at Line: {}, found invalid declaration inside class named: '{}'", f_EntryToken.m_SourceCodeLineNumber, f_ClassDec->ClassName.m_Value), "Parser");
                    return nullptr;
                }
            }
            break;
            case TokenType::Struct:
            {
                auto sv_NestedStruct = ParseStructDeclaration(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_NestedStruct)
                {
                    parser_logger->Error(format("Error at line: {}, invalid nested class definition found inside {}", f_EntryToken.m_SourceCodeLineNumber, f_ClassDec->ClassName.m_Value), "Parser");
                    return nullptr;
                }
            }
            break;
            case TokenType::Single:
            {
                parser_logger->Error(format("Error at line: {}, invalid 'single' statement inside class {} declaration", f_EntryToken.m_SourceCodeLineNumber, f_ClassDec->ClassName.m_Value), "Parser");
                parser_logger->Info(format("Tip: single cannot be used for nested class types, if a class needs to be single define it at global scope"), "Parser");
                return nullptr;
            }
            break;
            case TokenType::Class: //nested classes
            {
                auto sv_NestedClassDec = ParseClassDeclaration(fp_CurrentToken, fp_ProgramTokens, f_IsSingle);

                if (not sv_NestedClassDec)
                {
                    parser_logger->Error(format("Error at line: {}, invalid nested class definition found inside {}", f_EntryToken.m_SourceCodeLineNumber, f_ClassDec->ClassName.m_Value), "Parser");
                    return nullptr;
                }

                f_ClassDec->NestedClassDecs.push_back(std::move(sv_NestedClassDec));

                f_ClassDec->Modifiers |= f_CurrentAccessLevel; //add accessor level uwu

                f_IsSingle = false; //reset after used
            }
            break;
            case TokenType::Const:
            {
                auto sv_ConstStatement = ParseConstant(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_ConstStatement)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, invalid const definition found inside code block", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }
                else if (sv_ConstStatement->m_Domain == SyntaxNodeType::VarDeclaration)
                {
                    f_ClassDec->Fields.push_back(unique_dynamic_cast<VarDeclaration>(std::move(sv_ConstStatement)));
                }
                else if (sv_ConstStatement->m_Domain == SyntaxNodeType::FuncDeclaration)
                {
                    f_ClassDec->Methods.push_back(unique_dynamic_cast<FuncDeclaration>(std::move(sv_ConstStatement)));
                }
            }
            break;
            case TokenType::Private:
            {
                f_EntryToken = fp_CurrentToken;
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);

                if (fp_CurrentToken.m_Type != TokenType::DoubleDot)
                {
                    parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_CurrentAccessLevel = ModifierFlags::PRIVATE;
            }
            break;
            case TokenType::Protected:
            {
                f_EntryToken = fp_CurrentToken;
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);

                if (fp_CurrentToken.m_Type != TokenType::DoubleDot)
                {
                    parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_CurrentAccessLevel = ModifierFlags::PROTECTED;
            }
            break;
            case TokenType::Public:
            {
                f_EntryToken = fp_CurrentToken;
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);

                if (fp_CurrentToken.m_Type != TokenType::DoubleDot)
                {
                    parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_CurrentAccessLevel = ModifierFlags::PUBLIC;
            }
            break;
            case TokenType::CloseBracket:
                f_IsParsing = false;
                break;
            }
        }

        return f_ClassDec;
    }

    unique_ptr<InPlaceStructConstruction> //assumed entry token is '{'
        Parser::ParseStructConstruction
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        Token f_EntryToken = fp_CurrentToken;
        unique_ptr<InPlaceStructConstruction> f_StructConstruc = make_unique<InPlaceStructConstruction>();

        while (1)
        {
            fp_ProgramTokens.Peek(fp_CurrentToken);

            if (fp_CurrentToken.m_Type == TokenType::Dot) //INFO: used for the struct construction syntax in C/C++ where you can name the vars in a .var format
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift onto '.'
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);//now just looking for appropriate grammar

                if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)
                {
                    parser_logger->Error(format("Error at Line Number: {}, expected variable name but found: '{}' instead while parsing a named struct construction", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                    return nullptr;
                }

                Token f_VarName = fp_CurrentToken;
                fp_ProgramTokens.ShiftForward(fp_CurrentToken);

                if (fp_CurrentToken.m_Type != TokenType::Equals)
                {
                    parser_logger->Error(format("Error at Line Number: {}, expected '=' but found: '{}' instead while parsing a named struct construction", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                    return nullptr;
                }

                auto sv_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_RegExpr)
                {
                    parser_logger->Error(format("Error at Line Number: {}, unable to parse named in-place struct contructor call argument ;o;", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StructConstruc->ArgumentNames.push_back(f_VarName.m_Value);
                f_StructConstruc->Arguments.push_back(std::move(sv_RegExpr));
            }
            else if (fp_CurrentToken.m_Type == TokenType::Comma) //repeat loop uwu
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift onto comma
                continue;
            }
            else if (fp_CurrentToken.m_Type == TokenType::CloseBracket) //loop exit condition either it finds a close bracket or EOF uwu u choose
            {
                fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift onto closed bracket uwu
                break;
            }
            else //most basic case where its var x->mystruct = {1, 2, 3};
            {
                auto sv_RegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens); //avoid shift since the top of the loop overstepped a token at this point

                if (not sv_RegExpr)
                {
                    parser_logger->Error(format("Error at Line Number: {}, unable to parse struct contructor call argument ;o;", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_StructConstruc->Arguments.push_back(std::move(sv_RegExpr));

                //now parse reg expr should've ended on a comma or close bracket, and this function will decide what to do w those tokens using Peek() as well when the loop goes again
            }

        }

        return f_StructConstruc;
    }

    unique_ptr<StructDeclaration>
        Parser::ParseStructDeclaration
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        unique_ptr<StructDeclaration> f_StructDec = make_unique<StructDeclaration>();

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an user defined identifier eg. "class MyClass"

        if (fp_CurrentToken.m_Type != TokenType::UserIdentifier)//if it"s a proper name defintion then we can now proceed forwards, //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            parser_logger->Warning("Something bad happened while declaring your struct brother, did you forget or type your name wrong?", "Parser");
            return nullptr;
        }

        f_StructDec->StructName = fp_CurrentToken; //since the token is a user identifier it must be the name uwu
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward to look for an open paren

        if (fp_CurrentToken.m_Type != TokenType::OpenBracket) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            parser_logger->Warning("Something bad happened while declaring your class brother! Did you remember an open bracket?", "Parser");
            return nullptr;
        }

        bool f_IsParsing = true;

        while (f_IsParsing)
        {
            fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward since the current character is "{"

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

                f_StructDec->Members.push_back(std::move(sv_VariableDeclaration));
            }
            break;
            case TokenType::CloseBracket:
                f_IsParsing = false;
                break;
            default:
                parser_logger->Error(format("Error at Line Number: {}, found : '{}', when declaring variables inside a struct", to_string(fp_CurrentToken.m_SourceCodeLineNumber), fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }
        }

        return f_StructDec;
    }

    unique_ptr<ScopeDeclaration>
        Parser::ParseScopeDeclaration
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        unique_ptr<ScopeDeclaration> f_ScopeDec = make_unique<ScopeDeclaration>();

        return f_ScopeDec;
    }

    unique_ptr<ListDeclaration>
        Parser::ParseListDeclaration()
    {

        return nullptr;
    }

    unique_ptr<DictionaryDeclaration>
        Parser::ParseDictionaryDeclaration()
    {

        return nullptr;
    }

    unique_ptr<VarDeclaration> //returns true if it worked, false if it failed
        Parser::ParseVarDeclaration
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        Token f_EntryToken = fp_CurrentToken;

        unique_ptr<VarDeclaration> f_VarDec = make_unique<VarDeclaration>();

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward one token to check for variable name

        if (fp_CurrentToken.m_Type != TokenType::UserIdentifier) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}, expected a variable name but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            parser_logger->Warning("Something bad happened while declaring a variable brother! Try taking a look at the variable name definition", "Parser");
            return nullptr;
        }

        f_VarDec->Name = fp_CurrentToken;
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward one token to check for a type arrow

        if (fp_CurrentToken.m_Type != TokenType::TypeArrow)  //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}, expected type arrow '->' but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            parser_logger->Warning("Invalid symbol found after name-definition while declaring a variable brother! Try taking a look at the variable type-arrow definition", "Parser");
            return nullptr;
        }

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward one token to check for type and member access level

        if (not IsTypename(fp_CurrentToken)) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}, expected typename but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            parser_logger->Warning("Something bad happened while declaring a variable brother! Try taking a look at the variable type definition", "Parser");
            return nullptr;
        }

        f_VarDec->Type = fp_CurrentToken;
        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift forward one token to check for '='

        if (fp_CurrentToken.m_Type == TokenType::SemiDot) //for var decs like 'var x->int;' but every primitive type has a default initializer so w/e
        {
            return f_VarDec; //already done, just default value is nullptr uwu
        }

        if (fp_CurrentToken.m_Type != TokenType::Equals)
        {
            parser_logger->Error(format("Error at Line Number: {}, expected '=' during var declaration but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
            parser_logger->Warning(format("Found : '{}', when '=' was expected during definition of your variable named: '{}'", fp_CurrentToken.m_Value, f_VarDec->Name.m_Value), "Parser");
            return nullptr;
        }

        auto f_ParsedRegExpr = ParseRegularExpr(fp_CurrentToken, fp_ProgramTokens); //no shift here since the parseregexpr will do that for me uwu

        if (not f_ParsedRegExpr) //THROW ERROR
        {
            parser_logger->Error(format("Error at Line Number: {}, failed to parse variable value declaration", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            parser_logger->Warning("Invalid value found while declaring a variable brother! Try taking a look at the variable value definition", "Parser");
            return nullptr;
        }

        //now can move value expr into default value since if no compatible result is found, the function exits early with nullptr signaling failed parse
        f_VarDec->DefaultValue = std::move(f_ParsedRegExpr);

        fp_ProgramTokens.ShiftForward(fp_CurrentToken); //shift off expr, parse reg expr ends at the base case but it uses Peek() so only the expr is contained inside the Expr node uwu probably cleaner for compilation

        if (fp_CurrentToken.m_Type != TokenType::SemiDot)
        {
            parser_logger->Error(format("Error at Line Number: {}, YOU FORGOT A SEMICOLON AT THE END OF YOUR VARIABLE DECLARATION CLOD", fp_CurrentToken.m_SourceCodeLineNumber), "Parser");
            return nullptr;
        }

        return f_VarDec;
    }

    unique_ptr<StatementNode>
        Parser::ParseStatic
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        //Shift forward to look for a type token
        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

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

            f_StaticVarDec->Modifiers |= ModifierFlags::STATIC;

            return f_StaticVarDec;
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

            sv_StaticFuncDec->Modifiers |= ModifierFlags::STATIC;

            return sv_StaticFuncDec;
        }
        break;
        case TokenType::Const: //parse as regular var and just add const decorator on top of static
        {
            fp_ProgramTokens.Peek(fp_CurrentToken);

            if (fp_CurrentToken.m_Type == TokenType::Func)
            {
                auto sv_StaticConstFuncDec = ParseFuncDeclaration(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_StaticConstFuncDec)
                {
                    parser_logger->Error
                    (
                        format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)),
                        "Parser"
                    );

                    return nullptr;
                }

                sv_StaticConstFuncDec->Modifiers |= ModifierFlags::STATIC | ModifierFlags::CONSTANT;

                return sv_StaticConstFuncDec;
            }
            else if (fp_CurrentToken.m_Type == TokenType::Var)
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

                sv_StaticConstVarDec->Modifiers |= ModifierFlags::STATIC | ModifierFlags::CONSTANT;

                return sv_StaticConstVarDec;
            }
            else //TODO: add a way to convert from token type -> string for error messages
            {
                parser_logger->Error(format("Error at Line: {}, expected function or var definition but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }
        }
        break;
        default:
            parser_logger->Error
            (
                format("Parsing Error: expected var when using static, eg . 'static var ~~', but found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber),
                "Parser"
            );
        }

        return nullptr; // :(
    }

    unique_ptr<StatementNode>
        Parser::ParseConstant
        (
            Token fp_CurrentToken,
            VectorStream<Token>& fp_ProgramTokens
        )
    {
        //Shift forward to look for a type token
        fp_ProgramTokens.ShiftForward(fp_CurrentToken);

        switch (fp_CurrentToken.m_Type)
        {
        case TokenType::Var:
        {
            auto f_StaticVarDec = ParseVarDeclaration(fp_CurrentToken, fp_ProgramTokens);

            if (not f_StaticVarDec)
            {
                parser_logger->Error
                (
                    format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber),
                    "Parser"
                );

                return nullptr;
            }

            f_StaticVarDec->Modifiers |= ModifierFlags::CONSTANT;

            return f_StaticVarDec;
        }
        break;
        case TokenType::Func:
        {
            auto sv_StaticFuncDec = ParseFuncDeclaration(fp_CurrentToken, fp_ProgramTokens);

            if (not sv_StaticFuncDec)
            {
                parser_logger->Error
                (
                    format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber),
                    "Parser"
                );

                return nullptr;
            }

            sv_StaticFuncDec->Modifiers |= ModifierFlags::CONSTANT;

            return sv_StaticFuncDec;
        }
        break;
        case TokenType::Static: //parse as regular var and just add const decorator on top of static
        {
            fp_ProgramTokens.Peek(fp_CurrentToken);

            if (fp_CurrentToken.m_Type == TokenType::Func)
            {
                auto sv_StaticConstFuncDec = ParseFuncDeclaration(fp_CurrentToken, fp_ProgramTokens);

                if (not sv_StaticConstFuncDec)
                {
                    parser_logger->Error
                    (
                        format("Parsing Error: failed to parse variable declaration, found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, to_string(fp_CurrentToken.m_SourceCodeLineNumber)),
                        "Parser"
                    );

                    return nullptr;
                }

                sv_StaticConstFuncDec->Modifiers |= ModifierFlags::STATIC | ModifierFlags::CONSTANT;

                return sv_StaticConstFuncDec;
            }
            else if (fp_CurrentToken.m_Type == TokenType::Var)
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

                sv_StaticConstVarDec->Modifiers |= ModifierFlags::STATIC | ModifierFlags::CONSTANT;

                return sv_StaticConstVarDec;
            }
            else //TODO: add a way to convert from token type -> string for error messages
            {
                parser_logger->Error(format("Error at Line: {}, expected function or var definition but found: '{}' instead", fp_CurrentToken.m_SourceCodeLineNumber, fp_CurrentToken.m_Value), "Parser");
                return nullptr;
            }
        }
        break;
        default:
            parser_logger->Error
            (
                format("Parsing Error: expected var when using const, eg . 'const var ~~', but found: '{}' instead in source code at line: {}", fp_CurrentToken.m_Value, fp_CurrentToken.m_SourceCodeLineNumber),
                "Parser"
            );
        }

        return nullptr; // :(
    }

    bool
        Parser::ValidateFunctionReturnPaths()
    {

        return true;
    }

    //ValidateAST's main job is to check for things like scope errors, where a variable is being referenced outside its scope of definition
    bool
        Parser::ValidateAST(vector<StatementNode>& fp_ProgramStatements)
    {
        return true;
    }

    //////////////////////////////////////////////
    // Main Processing Function
    //////////////////////////////////////////////

    unique_ptr<Program>
        Parser::ConstructAST(VectorStream<Token>&& fp_ProgramTokens)
    {
        unique_ptr<Program> f_Program = make_unique<Program>();

        Token f_CurrentToken;
        Token f_EntryToken; //used for debugging 

        bool f_IsSingle = false; //used for tracking whether the class defintion we're in is a singleton or not

        while (not IsEOF(f_CurrentToken) and not fp_ProgramTokens.IsEmpty()) //this loop operates on every statement that lives on scope-depth = 0, everything else is branched like a tree from the 0th level
        {
            fp_ProgramTokens.ShiftForward(f_CurrentToken);
            f_EntryToken = f_CurrentToken;

            switch (f_CurrentToken.m_Type)
            {
            case TokenType::Func: //used for func and class method definitions
            {
                string sv_FuncName;

                fp_ProgramTokens.Peek(f_CurrentToken);

                if (f_CurrentToken.m_Type == TokenType::UserIdentifier)
                {
                    sv_FuncName = f_CurrentToken.m_Value;
                }

                unique_ptr<FuncDeclaration> sv_FunctionDefintion = ParseFuncDeclaration(f_CurrentToken, fp_ProgramTokens);

                if (not sv_FunctionDefintion)
                {
                    parser_logger->Error(format("Parsing Error at line:'{}', function named : '{}' declaration is invalid", f_EntryToken.m_SourceCodeLineNumber, sv_FuncName), "Parser");
                    return nullptr;
                }

                f_Program->ParsedScript.push_back(std::move(sv_FunctionDefintion));

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
                auto sv_ClassBlock = ParseClassDeclaration(f_CurrentToken, fp_ProgramTokens, f_IsSingle); //used for func args, var reassignment, class method access

                if (not sv_ClassBlock)
                {
                    parser_logger->Error(format("Parsing Error at line:'{}', couldn't parse class ;w;", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_Program->ParsedScript.push_back(std::move(sv_ClassBlock));
                f_IsSingle = false; //reset so that the next class definition doesn't get falsely labelled as a singleton
            }
            break;
            case TokenType::Struct: //define a struct
            {
                auto sv_StructBlock = ParseStructDeclaration(f_CurrentToken, fp_ProgramTokens); //used for func args, var reassignment, class method access

                if (not sv_StructBlock)
                {
                    parser_logger->Error(format("Parsing Error at line:'{}', couldn't parse struct ;w;", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_Program->ParsedScript.push_back(std::move(sv_StructBlock));
            }
            break;
            case TokenType::NameSpace:
            {
                unique_ptr<NamespaceDeclaration> sv_NameSpace = make_unique<NamespaceDeclaration>();

                fp_ProgramTokens.ShiftForward(f_CurrentToken); //look for namespace name uwu

                if (f_CurrentToken.m_Type != TokenType::UserIdentifier)
                {
                    parser_logger->Error(format("Parsing Error at line:'{}', expected name identifier when declaring a namespace but found: '{}' instead owo", f_EntryToken.m_SourceCodeLineNumber, f_CurrentToken.m_Value), "Parser");
                    return nullptr;
                }

                sv_NameSpace->m_Name = f_CurrentToken;

                f_Program->ParsedScript.push_back(std::move(sv_NameSpace));
            }
            break;
            case TokenType::Include:
            {
                unique_ptr<IncludeStatement> sv_Include = make_unique<IncludeStatement>();
                //Shift forward to look for a string token
                fp_ProgramTokens.ShiftForward(f_CurrentToken);

                if (f_CurrentToken.m_Type != TokenType::StringLiteral)
                {
                    parser_logger->Error
                    (
                        format("Parsing Error: expected string for include but found: '{}' instead in source code at line: {}", f_CurrentToken.m_Value, f_CurrentToken.m_SourceCodeLineNumber),
                        "Parser"
                    );

                    return nullptr;
                }

                sv_Include->m_IncludePath = f_CurrentToken;
                f_Program->ParsedScript.push_back(std::move(sv_Include));

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
                        format("Parsing Error: Improper grammar used when defining global variable, found: '{}' instead in source code at line: {}", f_CurrentToken.m_Value, f_CurrentToken.m_SourceCodeLineNumber),
                        "Parser"
                    );
                }

                f_Program->ParsedScript.push_back(std::move(sv_GlobalVariable)); //global var owo
            }
            break;
            case TokenType::Static:
            {
                auto sv_StaticStatement = ParseStatic(f_CurrentToken, fp_ProgramTokens);

                if (not sv_StaticStatement)
                {
                    parser_logger->Error(format("Parsing Error at line: {}, couldn't parse static whatever ", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                f_Program->ParsedScript.push_back(std::move(sv_StaticStatement)); //global static var
            }
            break; //should shiftforward at top of loop at work fine uwu
            case TokenType::Const:
            {
                unique_ptr<StatementNode> sv_ConstantStatement = ParseConstant(f_CurrentToken, fp_ProgramTokens);

                if (not sv_ConstantStatement)
                {
                    parser_logger->Error(format("Error at Line Number: {}, invalid const definition", f_EntryToken.m_SourceCodeLineNumber), "Parser");
                    return nullptr;
                }

                if (sv_ConstantStatement->m_Domain == SyntaxNodeType::FuncDeclaration)
                {
                    auto sv_RecastedFuncDec = unique_dynamic_cast<FuncDeclaration>(std::move(sv_ConstantStatement));

                    parser_logger->Error(format("Syntax Error at Line Number: {}, function named : '{}' cannot be declared 'const' since it isn't a member of a class", f_EntryToken.m_SourceCodeLineNumber, sv_RecastedFuncDec->m_FuncName.m_Value), "Parser");
                    return nullptr;
                }

                f_Program->ParsedScript.push_back(std::move(sv_ConstantStatement)); //global const var
            }
            break;
            case TokenType::ENDF:
                break;
            default:
            {
                parser_logger->Error
                (
                    format("Parsing Error: Improper grammar used in global scope! found: '{}' in source code at line: {} ", f_CurrentToken.m_Value, f_CurrentToken.m_SourceCodeLineNumber),
                    "Parser"
                );

                return nullptr;
            }
            break; //OwO
            }
        }
        //end of while-switch loop

        return f_Program;
    }
}