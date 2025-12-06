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

#include "Lexer.h"

#include <array>
#include <vector>
#include <unordered_map>
#include <memory>


namespace BongoJam {
    //////////////////////////////////////// SyntaxNodes Used for Parsing ////////////////////////////////////////

    enum class SyntaxNodeType
    {
        //////////////////// Compiler Specific ////////////////////

        NameSpace,
        IncludeStatement,

        //////////////////// User Declarations ////////////////////

        VarDeclaration,

        IfDeclaration, //else(if) statements are only found under an if-statement so we're good not to explicitly type for it
        ElseIfDeclaration,
        ElseDeclaration,

        FuncDeclaration,

        WhileLoopDeclaration,
        ForLoopDeclaration,

        ClassDeclaration,
        StructDeclaration,
        ScopeDeclaration,

        TryCatchDeclaration,

        StatementBlock,

        InPlaceStructConstruction,

        //////////////////// Built-in Class Types ////////////////////

        ListDeclaration,
        DictionaryDeclaration,

        //////////////////// Expressions ////////////////////

        Expr,

        SingleValueExpr,

        BinaryOperationExpr,
        UnaryOperatorExpr,

        ParenExpr,

        FunctionCallExpr,
        IdentifierExpr,

        ClassCallExpr,
        StructCallExpr,

        VariableReassignmentExpr,
        ContainerIndexedAccessExpr,

        FmtdStringExpr,

        //////////////////// Included Functions ////////////////////

        StandardFunctionCallExpr,

        //////////////////// Conditional Child Statements ////////////////////

        BreakSubStatement,
        ContinueSubStatement,
        ReturnSubStatement,

        //////////////////// Default type uwu ////////////////////
        None
    };

    const map<string, TokenType> STANDARD_FUNCTIONS =
    {
        //////////////////// Included Functions ////////////////////

        { "print", TokenType::Print},
        { "input", TokenType::Input},
        { "clock", TokenType::Clock},
        { "size_of", TokenType::SizeOf},
        { "type_of", TokenType::TypeOf},

        { "to_string", TokenType::ToString},
        
        //////////////////// Math ////////////////////

        {"round_down", TokenType::Floor},
        {"round_up", TokenType::Ceiling},

        { "sqrt", TokenType::SquareRoot},
        { "pow", TokenType::Power},

        { "sin", TokenType::Sin},
        { "sinh", TokenType::Sinh},
        { "cos", TokenType::Cos},
        { "cosh", TokenType::Cosh},
        { "tan", TokenType::Tan},
        { "tanh", TokenType::Tanh},
        { "arccos", TokenType::ArcCos},
        { "arcsin", TokenType::ArcSin},
        { "arctan", TokenType::ArcTan},

        { "exp", TokenType::Exp},
        { "log", TokenType::NaturalLog},
        { "factorial", TokenType::Factorial},

        //////////////////// Error Handling ////////////////////
        
        {"panic", TokenType::Panic}, //stops program execution and prints a message
        {"static_assert", TokenType::StaticAssert}, //compile time assert, gest thrown out after compilation
        {"assert", TokenType::Assert}, //runtime assert, bundled into bytecode
    };

    //////////////////////////////////////////////
    // Utility Functions
    //////////////////////////////////////////////

    struct StatementNode
    {
        SyntaxNodeType m_Domain = SyntaxNodeType::None;
        virtual ~StatementNode() = default;

        // (Optional) base constructor
        StatementNode(SyntaxNodeType domain) : m_Domain(domain) {}

        StatementNode() = default;
    };

    //////////////////////////////////////////////
    // Expression Definitions
    //////////////////////////////////////////////

    struct Expr : public StatementNode //it twas not a cannon and ur dumb and lazy about learning variants uwu
    {
        Expr() : StatementNode(SyntaxNodeType::Expr) {}

        TokenType EvaluatesTo = TokenType::NO_TOKEN_VALUE;
    };

    struct ParenExpr : public Expr //idk if ill use these for function definition/call args as well, which would also translate to classes naturally
    {
        ParenExpr() { m_Domain = SyntaxNodeType::ParenExpr; }
        unique_ptr<Expr> Inside; //inside evals to a tree or singlevalexpr
        Token Decorator; //used primarily for @bgn or for a prefix op like negative or not
    };

    struct SingleValueExpr : public Expr
    {
        Token m_Value;

        Token Decorator; //used primarily for @bgn or for a prefix op like negative or not

        explicit SingleValueExpr(Token fp_ValueToken, Token fp_Decorator = Token()) 
        {
            m_Domain = SyntaxNodeType::SingleValueExpr; 
            m_Value = fp_ValueToken;
            Decorator = fp_Decorator;
        }

        SingleValueExpr() = default;
    };

    struct BinaryOperationExpr : public Expr
    {
        //meant to be overriden
        BinaryOperationExpr()
        { 
            m_Domain = SyntaxNodeType::BinaryOperationExpr; 
        }

        BinaryOperationExpr(Token fp_Operator, unique_ptr<Expr>&& fp_First, unique_ptr<Expr>&& fp_Second) 
        { 
            m_Domain = SyntaxNodeType::BinaryOperationExpr;

            First = move(fp_First);
            Second = move(fp_Second);

            m_Operator = fp_Operator;
        }

        Token m_Operator;
        unique_ptr<Expr> First, Second;
    };

    struct UnaryOperatorExpr : public Expr
    {
        UnaryOperatorExpr() { m_Domain = SyntaxNodeType::UnaryOperatorExpr; }

        UnaryOperatorExpr(Token fp_Operator, unique_ptr<Expr>&& fp_Value)
        {
            m_Domain = SyntaxNodeType::BinaryOperationExpr;
            Value = move(fp_Value);
            m_Operator = fp_Operator;
        }

        Token m_Operator;
        unique_ptr<Expr> Value;
    };

    struct StatementBlock: public StatementNode //used as a scope
    {
        StatementBlock() : StatementNode(SyntaxNodeType::StatementBlock) {}

        vector<unique_ptr<StatementNode>> CodeBody;

        //WARNING THIS BREAKS EVERYTHING FOR SOME REASON LMFAO
        //map<string, Token> SymbolTable; //symbols defined inside a function, important for functions that call external src files which is the case most of the time imo
    };

    struct IncludeStatement :public StatementNode
    {
        IncludeStatement() : StatementNode(SyntaxNodeType::IncludeStatement) {}
        Token m_IncludePath;
    };

    struct NamespaceDeclaration : public StatementNode
    {
        NamespaceDeclaration(): StatementNode(SyntaxNodeType::NameSpace) {}
        Token m_Name;
    };

    struct ReturnStatement : public StatementNode
    {
        ReturnStatement() : StatementNode(SyntaxNodeType::ReturnSubStatement) {}
        unique_ptr<Expr> ReturnValue = nullptr;
    };

    struct BreakStatement : public StatementNode
    {
        BreakStatement() : StatementNode(SyntaxNodeType::BreakSubStatement) {} //no value needed besides the tag since its always just "break" uwu atm at least owo
    };

    struct ContinueStatement : public StatementNode
    {
        ContinueStatement() : StatementNode(SyntaxNodeType::ContinueSubStatement) {}
    };

    struct TryCatchDeclaration : public StatementNode //can i break from inside a try statement mid loop uwu les try
    {
        TryCatchDeclaration() : StatementNode(SyntaxNodeType::TryCatchDeclaration) {}

        unique_ptr<StatementBlock> TryBlock = nullptr;

        unique_ptr<StatementBlock> CatchBlock = nullptr;
        unique_ptr<Expr> CatchCondition = nullptr;

    };

    //=========================================================================================== Modifier Flags for Variables/Classes/Funcs/Structs ===========================================================================================//

    enum ModifierFlags : uint8_t
    {
        NONE = 0,
        PRIVATE = 1 << 0,
        PROTECTED = 1 << 1,
        PUBLIC = 1 << 2, //pretty sure i can just remove this since public is implied and any change in visibility is explicit
        STATIC = 1 << 3,
        CONSTANT = 1 << 4,
        SINGLE = 1 << 5 //singleton class or struct, semantically it means no new object can be created so just deletes it and only creates one at program start, cannot be used as a field
    };

    //=========================================================================================== Variable assignment and re-assignment ===========================================================================================//

    struct VarDeclaration : public Expr //gets its symbol resolved at compilation
    {
        VarDeclaration() { m_Domain = SyntaxNodeType::VarDeclaration; }

        Token Name;
        Token Type;

        //DANGER: this value always needs to be null checked since var declaration is used for vars that have a user defined default value, if not a default value will ALWAYS be assigned to any primitive type since it takes memory might as well store a val idrc ab the clock cycles
        unique_ptr<Expr> DefaultValue = nullptr; //could be an expression like when i default a unique ptr using  ptr = make_unique<>() in a class field declaration

        uint8_t Modifiers = NONE; //static or const or access level uwu
    };

    struct VariableReassignmentExpr : public Expr
    {
        VariableReassignmentExpr() { m_Domain = SyntaxNodeType::VariableReassignmentExpr; }
        unique_ptr<Expr> VariableName; //IMPORTANT: this is an expr for chained var calls and index access eg. 'myClass.field or myList[69]'
        Token Operator;
        unique_ptr<Expr> NewValue = nullptr;
    };

    //=========================================================================================== Special User Identifier Based Calls ===========================================================================================//
    
    struct InPlaceStructConstruction : public Expr
    {
        InPlaceStructConstruction() { m_Domain = SyntaxNodeType::InPlaceStructConstruction; }
        vector<unique_ptr<Expr>> Arguments; //indices here and in argument names should match, just argument names is optional so no map uwu
        vector<string> ArgumentNames; //string for ".field =" in place construction
    };

    struct ContainerIndexedAccessExpr : public Expr //myList[0]
    {
        ContainerIndexedAccessExpr() { m_Domain = SyntaxNodeType::ContainerIndexedAccessExpr; }
        Token ContainerName;

        unique_ptr<Expr> DesiredIndex = nullptr; //can be a var expr or a numeric expr or a combination as a mathematical expr
        unique_ptr<Expr> ChainedExpr = nullptr;

        Token Decorator; //used primarily for @bgn or w/e
    };

    struct FunctionCallExpr : public Expr  //idk how to get maybe after parsing we do a grammar check uwu everything could be spelt right but not make perfect sense
    {
        explicit FunctionCallExpr() { m_Domain = SyntaxNodeType::FunctionCallExpr; }
        
        Token FuncName;

        vector<unique_ptr<Expr>> Arguments; //vector cause multiple arguments unknown size, expr because it could get crazy uwu

        //WARNING: ChainedIdentifier can be null so null checks are MANDATORY
        unique_ptr<Expr> ChainedIdentifier = nullptr; //in a call chain this is ...MyFunc().MyClass.................

        Token Decorator; //used primarily for @bgn or w/e
    };

    struct IdentifierExpr : public Expr
    {
        IdentifierExpr(Token fp_IdentifierToken, Token fp_Decorator = Token()) 
        { 
            m_Domain = SyntaxNodeType::IdentifierExpr; 
            Identifier = fp_IdentifierToken;
            Decorator = fp_Decorator;
        }

        IdentifierExpr() { m_Domain = SyntaxNodeType::IdentifierExpr; }

        Token Identifier;
        unique_ptr<Expr> ChainedExpr = nullptr; //reg expr because could be chained to one of the other two expressions above

        Token Decorator; //used primarily for @bgn or w/e
    };

    //=========================================================================================== Control Flow ===========================================================================================//


    struct ElseDeclaration : public StatementBlock
    {
        ElseDeclaration() { m_Domain = SyntaxNodeType::ElseDeclaration; }
    };

    struct IfDeclaration : public StatementBlock
    {
        IfDeclaration() { m_Domain = SyntaxNodeType::IfDeclaration; }

        unique_ptr<Expr> m_Condition = nullptr;; //needs to be a vector since and and or's are a thing

        vector<unique_ptr<IfDeclaration>> m_ElseIfStatements; //unique ptr since its easier w ptr ownership semantics and avoiding rleasing the ptr and copying the dereference which would cause a problem w the condition var
        unique_ptr<ElseDeclaration> m_ElseStatement = nullptr;
    };

    struct FuncArgument
    {
        Token Name;
        Token Type;
        Token DefaultValue;
    };

    struct FuncDeclaration : public StatementBlock
    {
        FuncDeclaration() { m_Domain = SyntaxNodeType::FuncDeclaration; }
        Token m_FuncName;
        Token m_FuncReturnType;

        vector<FuncArgument> Arguments;

        uint8_t Modifiers = NONE;
    };


    struct WhileLoopDeclaration : public StatementBlock
    {
        WhileLoopDeclaration() { m_Domain = SyntaxNodeType::WhileLoopDeclaration; }
        unique_ptr<Expr> m_Condition = nullptr; //needs to be a vector since and and or's are a thing
    };

    struct ForLoopDeclaration : public StatementBlock
    {
        ForLoopDeclaration() { m_Domain = SyntaxNodeType::ForLoopDeclaration; }
    };


    struct ClassDeclaration : public StatementNode //not a block node since a class is just variables + functions and a name for symbol resolution
    {
        ClassDeclaration() : StatementNode(SyntaxNodeType::ClassDeclaration) {}

        Token ClassName;
        uint8_t Modifiers = ModifierFlags::NONE;

        unordered_map<string, Token> SymbolTable; //symbols defined inside a function, important for functions that call external src files which is the case most of the time imo

        vector<unique_ptr<FuncDeclaration>> Constructors; //list of constructors for declared class
        vector<unique_ptr<FuncDeclaration>> Methods; //list of methods used in class

        vector<unique_ptr<VarDeclaration>> Fields; //needs to be a unique ptr since var declaration holds a unique ptr

        vector<unique_ptr<ClassDeclaration>> NestedClassDecs;

    };

    //structs are classes without methods, excluding the constructor(s)
    //structs are just used as generic data containers, where operators are defined on it that dictate how this struct interacts with other of its or other types
    struct StructDeclaration : public StatementNode
    {
        StructDeclaration() : StatementNode(SyntaxNodeType::StructDeclaration) {}

        Token StructName;
        uint8_t Modifiers = ModifierFlags::NONE; //the struct doesnt have access levels for its members but the struct could be defined within a class uwu

        vector<FuncDeclaration> Constructors; //list of constructors for declared class, FuncArgument can be stack allocated since it has no unique ptrs uwu

        //needs to be a unique ptr since var declaration holds a unique ptr
        vector <unique_ptr<VarDeclaration>> Members; //all struct members are public always so no accessor levels, and structs dont use inheritance uwu just aliasing
    };

    struct ScopeDeclaration : public StatementBlock
    {
        ScopeDeclaration() { m_Domain = SyntaxNodeType::ScopeDeclaration; }
    };

    struct ListDeclaration : public StatementNode
    {
        ListDeclaration() : StatementNode(SyntaxNodeType::ListDeclaration) {}
    };

    struct DictionaryDeclaration : public StatementNode
    {
        DictionaryDeclaration() : StatementNode(SyntaxNodeType::DictionaryDeclaration) {}
    };

    //////////////////////////////////////////////
    // Numeric Expressions
    //////////////////////////////////////////////

    struct FmtdStringExpr : public Expr //this expr is meant to be traversed in order since f"hello {plant} i love you" gets parsed as three tokens
    {
        FmtdStringExpr() { m_Domain = SyntaxNodeType::FmtdStringExpr;}

        //this list has single val expr for the string islands and any expr for the insert var
        //used for tracking exprs used inside the formatted string arg style, ordering matters here, index 0 --> first value inserted to string at first slot
        vector<unique_ptr<Expr>> FullString; 
    };

    //////////////////////////////////////// Primitive Value Type Enum ////////////////////////////////////////

    struct Program
    {
        //first function should be the very first function, defined in the very top level of the import tree
        vector<unique_ptr<StatementNode>> ParsedScript; //contains all defined functions inside the script
    };
}