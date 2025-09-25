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

#include <variant>
#include <array>
#include <vector>
#include <unordered_map>

#define BONGO_PRIMITIVE_VALUE variant \
< \
    int8_t, int16_t, int32_t, int64_t, \
    uint8_t, uint16_t, uint32_t, uint64_t, \
    float, double, \
    char, string \
>

namespace BongoJam {
    //////////////////////////////////////// SyntaxNodes Used for Parsing ////////////////////////////////////////

    enum class SyntaxNodeType
    {
        //////////////////// Compiler Specific ////////////////////

        NameSpace,
        IncludeStatement,

        //////////////////// User Declarations ////////////////////

        VarDeclaration,
        FieldDeclaration,
        VariableDeclaration,

        IfDeclaration, //else(if) statements are only found under an if-statement so we're good not to explicitly type for it
        ElseIfDeclaration,
        ElseDeclaration,

        FuncDeclaration,
        MethodDeclaration,

        WhileLoopDeclaration,
        ForLoopDeclaration,

        ClassDeclaration,
        StructDeclaration,
        ScopeDeclaration,

        StatementBlock,

        //////////////////// Built-in Class Types ////////////////////

        ListDeclaration,
        DictionaryDeclaration,

        //////////////////// Expressions ////////////////////

        Expr,

        SingleValueExpr,

        BinaryOperationExpr,
        UnaryOperatorExpr,

        AdditionExpr,
        MultiplicationExpr,

        SubtractionExpr,
        DivisionExpr,

        ModuloExpr,

        PlusEqualsExpr,
        MinusEqualsExpr,

        DivEqualsExpr,
        MultEqualsExpr,

        ModuloEqualsExpr,

        OpenParenExpr,

        GreaterThanExpr,
        GreaterThanOrEqualsExpr,

        LesserThanExpr,
        LesserThanOrEqualsExpr,

        StrictlyEqualsExpr,

        AndExpr,
        OrExpr,
        NotExpr,

        MethodCallExpr,
        FunctionCallExpr,

        ClassCallExpr,
        StructCallExpr,

        VariableReassignmentExpr,
        FieldReassignmentExpr,

        //////////////////// Access Levels ////////////////////

        PublicAccess,
        ProtectedAccess,
        PrivateAccess,

        //////////////////// Included Functions ////////////////////

        StandardFunction,
        PrintFunction,

        //////////////////// Conditional Child Statements ////////////////////

        BreakSubStatement,
        ContinueSubStatement,
        ReturnSubStatement,

        //////////////////// Default type uwu ////////////////////
        None
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

    struct OpenParenExpr : public Expr //idk if ill use these for function definition/call args as well, which would also translate to classes naturally
    {
        OpenParenExpr() { m_Domain = SyntaxNodeType::OpenParenExpr; }
        vector<unique_ptr<Expr>> Inside;
    };

    struct SingleValueExpr : public Expr
    {
        Token m_Value;

        SingleValueExpr(Token fp_ValueToken) 
        {
            m_Domain = SyntaxNodeType::SingleValueExpr; 
            m_Value = fp_ValueToken;
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

    struct NameSpaceBlock : public StatementBlock
    {
        NameSpaceBlock() { m_Domain = SyntaxNodeType::NameSpace; }
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

    //////////////////////////////////////////////
    // Variable assignment and re-assignment
    //////////////////////////////////////////////

    struct VarDeclaration : public Expr //gets its symbol resolved at compilation
    {
        VarDeclaration() { m_Domain = SyntaxNodeType::VarDeclaration; }

        Token Name;
        Token Type;

        unique_ptr<Expr> DefaultValue = nullptr; //could be an expression like when i default a unique ptr using  ptr = make_unique<>() in a class field declaration

        bool IsStatic = false;
        bool IsConst = false;
    };


    struct FieldDeclaration : public VarDeclaration
    {
        FieldDeclaration() { m_Domain = SyntaxNodeType::FieldDeclaration; }

        Token m_AccessLevel; //used for tracking private, public, or protected
    };

    struct VariableReassignmentExpr : public Expr
    {
        VariableReassignmentExpr() { m_Domain = SyntaxNodeType::VariableReassignmentExpr; }
    };

    struct FieldReassignmentExpr : public VariableReassignmentExpr
    {
        FieldReassignmentExpr() { m_Domain = SyntaxNodeType::FieldReassignmentExpr; }
    };

    //////////////////////////////////////////////
    // Declarations Involving Multiple Expressions
    //////////////////////////////////////////////

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

        bool IsStatic = false;
        bool IsConstant = false;
    };

    struct MethodDeclaration : public FuncDeclaration
    {
        MethodDeclaration() { m_Domain = SyntaxNodeType::MethodDeclaration; }
        Token m_AccessLevel; //used for tracking private, public, or protected
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

        unordered_map<string, Token> SymbolTable; //symbols defined inside a function, important for functions that call external src files which is the case most of the time imo

        vector<FuncDeclaration> Constructors; //list of constructors for declared class
        vector<FuncDeclaration> Methods; //list of methods used in class

        vector<unique_ptr<FieldDeclaration>> Fields; //needs to be a unique ptr since var declaration holds a unique ptr

    };

    //structs are classes without methods, excluding the constructor(s)
    //structs are just used as generic data containers, where operators are defined on it that dictate how this struct interacts with other of its or other types
    struct StructDeclaration : public StatementNode
    {
        StructDeclaration() : StatementNode(SyntaxNodeType::StructDeclaration) {}

        Token StructName;
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
    // Standard Function Types
    //////////////////////////////////////////////
    

    struct PrintFunction : public Expr
    {
        PrintFunction() { m_Domain = SyntaxNodeType::PrintFunction; }

        array<unique_ptr<Expr>, 2> m_FuncArgs; //will only ever be two arguments for now, adding a f"" format feature cause i like that
    };

    //////////////////////////////////////////////
    // Numeric Expressions
    //////////////////////////////////////////////


    struct AdditionExpr : public BinaryOperationExpr
    {
        AdditionExpr() { m_Domain = SyntaxNodeType::AdditionExpr; }
    };


    struct SubtractionExpr : public BinaryOperationExpr
    {
        SubtractionExpr() { m_Domain = SyntaxNodeType::SubtractionExpr; }
    };


    struct MultiplicationExpr : public BinaryOperationExpr
    {
        MultiplicationExpr() { m_Domain = SyntaxNodeType::MultiplicationExpr; }
    };


    struct DivisionExpr : public BinaryOperationExpr
    {
        DivisionExpr() { m_Domain = SyntaxNodeType::DivisionExpr; }
    };


    struct ModuloExpr : public BinaryOperationExpr
    {
        ModuloExpr() { m_Domain = SyntaxNodeType::ModuloExpr; }
    };


    struct PlusEqualsExpr : public BinaryOperationExpr
    {
        PlusEqualsExpr() { m_Domain = SyntaxNodeType::PlusEqualsExpr; }
    };


    struct MinusEqualsExpr : public BinaryOperationExpr
    {
        MinusEqualsExpr() { m_Domain = SyntaxNodeType::MinusEqualsExpr; }
    };


    struct MultEqualsExpr : public BinaryOperationExpr
    {
        MultEqualsExpr() { m_Domain = SyntaxNodeType::MultEqualsExpr; }
    };


    struct DivEqualsExpr : public BinaryOperationExpr
    {
        DivEqualsExpr() { m_Domain = SyntaxNodeType::DivEqualsExpr; }
    };


    struct ModuloEqualsExpr : public BinaryOperationExpr
    {
        ModuloEqualsExpr() { m_Domain = SyntaxNodeType::ModuloEqualsExpr; }
    };

    struct MethodCallExpr : public Expr
    {
        MethodCallExpr() { m_Domain = SyntaxNodeType::MethodCallExpr; }
        Token m_MethodName;
        Token m_ReturnType;
        vector<Token> m_MethodArgs;
    };


    struct FunctionCallExpr : public Expr
    {
        FunctionCallExpr() { m_Domain = SyntaxNodeType::FunctionCallExpr; }
        Token m_FuncName;
        Token m_ReturnType;
        vector<Token> m_FuncArgs;
    };


    struct ClassCallExpr : public Expr //used for: "let x->myClass = new myClass();"
    {
        ClassCallExpr() { m_Domain = SyntaxNodeType::ClassCallExpr; }

        bool m_IsHeapAllocated = false;
    };


    struct StructCallExpr : public Expr //used for: "var x->myStruct = new myStruct();"
    {
        StructCallExpr() { m_Domain = SyntaxNodeType::StructCallExpr; }

        bool m_IsHeapAllocated = false;
    };

    //////////////////////////////////////////////
    // Boolean Expressions
    //////////////////////////////////////////////


    struct GreaterThanExpr : public BinaryOperationExpr
    {
        GreaterThanExpr() { m_Domain = SyntaxNodeType::GreaterThanExpr; }

    };


    struct GreaterThanOrEqualsExpr : public BinaryOperationExpr
    {
        GreaterThanOrEqualsExpr() { m_Domain = SyntaxNodeType::GreaterThanOrEqualsExpr; }

    };


    struct LesserThanExpr : public BinaryOperationExpr
    {
        LesserThanExpr() { m_Domain = SyntaxNodeType::LesserThanExpr; }

    };


    struct LesserThanOrEqualsExpr : public BinaryOperationExpr
    {
        LesserThanOrEqualsExpr() { m_Domain = SyntaxNodeType::LesserThanOrEqualsExpr; }

    };


    struct StrictlyEqualsExpr : public BinaryOperationExpr
    {
        StrictlyEqualsExpr() { m_Domain = SyntaxNodeType::StrictlyEqualsExpr; }

    };


    struct AndExpr : public BinaryOperationExpr
    {
        AndExpr() { m_Domain = SyntaxNodeType::AndExpr; }

    };


    struct OrExpr : public BinaryOperationExpr
    {
        OrExpr() { m_Domain = SyntaxNodeType::OrExpr; }

    };


    struct NotExpr : public UnaryOperatorExpr
    {
        NotExpr() { m_Domain = SyntaxNodeType::NotExpr; }

    };

    //////////////////////////////////////// Primitive Value Type Enum ////////////////////////////////////////

    struct Program
    {
        //first function should be the very first function, defined in the very top level of the import tree
        vector<unique_ptr<StatementNode>> ParsedScript; //contains all defined functions inside the script
    };

}