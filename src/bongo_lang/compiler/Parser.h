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

#include "AST.h"

namespace BongoJam {

    template<typename T>
    [[nodiscard]] static inline unique_ptr<T> 
        unique_dynamic_cast(unique_ptr<StatementNode>&& fp_BasePtr)
    {
        T* f_DerivedType = dynamic_cast<T*>(fp_BasePtr.release());
        return unique_ptr<T>(f_DerivedType);
    }
    
    class Parser
    {
    private:
        shared_ptr<Logger> parser_logger = nullptr; //borrowed from the compiler

    public:
        Parser(shared_ptr<Logger> fp_CompilerLogger);

        ~Parser() = default;

    private:
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
            ParseNumber
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= String Literals =================================================================================================//

        unique_ptr<Expr>
            ParseString //assuming the current token == StringLiteral
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                Token fp_Decorator = Token()
            );

//================================================================================================= User Identifiers =================================================================================================//

        //this function is used to deal with user defined tokens relating to lines of code like "myVar = newVal;" or "myClass.myFunc();" or "myFunc();"		    handled by ParseNumber()
        //we also deal with expressions formed within method or function calls, so this method will return an expression ending with ';' or ',' eg. myFunc(1, 3 + otherFunc(), otherFunc() * 2);
        //																																					called					      called
        unique_ptr<Expr>
            ParseUserIdentifier //called when current token = user identifier, so it handles shifting
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                Token fp_Decorator = Token()
            );

//================================================================================================= Formatted String Literals =================================================================================================//

        unique_ptr<FmtdStringExpr> //assuming being called on FormattedStringStart, idk ab FormattedStringEnd
            ParseFormattedString
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Parenthesis =================================================================================================//

        unique_ptr<ParenExpr>
            ParseOpenParen //should always return as a full unit of "(" + ")"
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Regular Expressions =================================================================================================//

        //used for parsing any expr, and types it accordingly------ the assumed entry is a caller that is optimistic about the result being an expression so as a result the current token is right before said expression uwu
        unique_ptr<Expr>
            ParseRegularExpr
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= General Statement Blocks =================================================================================================//
        
        unique_ptr<StatementBlock>
            ParseStatementBlock
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                bool fp_IsInLoop
            );

//================================================================================================= Try/Catch =================================================================================================//

        unique_ptr<TryCatchDeclaration>
            ParseTryCatchBlock //catch should come immediately after try always 
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                bool fp_IsInLoop
            );

//================================================================================================= If/Else =================================================================================================//

        unique_ptr<IfDeclaration>
            ParseIfBlock
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                bool fp_IsInLoop,
                bool fp_IsInsideElseIf = false
            );

        unique_ptr<ElseDeclaration>
            ParseElseBlock
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                bool fp_IsInLoop
            );

//================================================================================================= While Loop =================================================================================================//

        unique_ptr<WhileLoopDeclaration>
            ParseWhileBlock
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= For Loop =================================================================================================//

        unique_ptr<ForLoopDeclaration>
            ParseForBlock
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Function Return =================================================================================================//

        unique_ptr<ReturnStatement>
            ParseReturnStatement
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Function Declaration =================================================================================================//

        unique_ptr<FuncDeclaration>
            ParseFuncDeclaration
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Function Call =================================================================================================//

        unique_ptr<FunctionCallExpr> //Used for functions, standard functions and class constructors
            ParseFunctionCall
            (
                const Token fp_NameToken,
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Class Declaration =================================================================================================//
        
        unique_ptr<ClassDeclaration>
            ParseClassDeclaration
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens,
                bool fp_IsSingle
            );
//================================================================================================= Structs =================================================================================================//

        unique_ptr<InPlaceStructConstruction> //assumed entry token is '{'
            ParseStructConstruction
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

        unique_ptr<StructDeclaration>
            ParseStructDeclaration
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= Scope Declaration =================================================================================================//

        unique_ptr<ScopeDeclaration>
            ParseScopeDeclaration
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

//================================================================================================= List and Dictionary =================================================================================================//

        unique_ptr<ListDeclaration>
            ParseListDeclaration();

        unique_ptr<DictionaryDeclaration>
            ParseDictionaryDeclaration();

//================================================================================================= Variable Declaration =================================================================================================//

        unique_ptr<VarDeclaration> //returns true if it worked, false if it failed
            ParseVarDeclaration
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

        unique_ptr<StatementNode>
            ParseStatic
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

        unique_ptr<StatementNode>
            ParseConstant
            (
                Token fp_CurrentToken,
                VectorStream<Token>& fp_ProgramTokens
            );

        bool
            ValidateFunctionReturnPaths();

        //ValidateAST's main job is to check for things like scope errors, where a variable is being referenced outside its scope of definition, type checking
        unique_ptr<TranslationUnit>
            ValidateAST(unique_ptr<TranslationUnit>&& fp_ProgramStatements);
    
    public:
        //////////////////////////////////////////////
        // Main Processing Function
        //////////////////////////////////////////////

        unique_ptr<TranslationUnit>
            ConstructAST(VectorStream<Token>&& fp_ProgramTokens);
    };
}