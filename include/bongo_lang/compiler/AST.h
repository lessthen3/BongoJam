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
        //////////////////// User Declarations ////////////////////

        VarDeclaration,
        FieldDeclaration,
        IfDeclaration, //else(if) statements are only found under an if-statement so we're good not to explicitly type for it

        VariableDeclaration,

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

        BracketedExpr,

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

        //////////////////// User Defined Names ////////////////////

        ClassName,
        StructName,
        StringName,
        IntName,
        FloatName,
        BoolName,
        FuncName,

        //////////////////// Dependent on Parent Statements ////////////////////

        BreakSubStatement,
        ElseIfDeclaration,
        ElseDeclaration,
        ReturnSubStatement,
        ImportStatement,

        None
    };

    struct PrimitiveValue
    {
        Token m_Token;
        BONGO_PRIMITIVE_VALUE Value;
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
    };

    struct SingleValueExpr : public Expr
    {
        PrimitiveValue Value;

        SingleValueExpr(Token fp_ValueToken) 
        {
            m_Domain = SyntaxNodeType::SingleValueExpr; 
            Value.m_Token = fp_ValueToken;
        }

        SingleValueExpr() = default;
    };

    struct BracketedExpr : public Expr
    {
        BracketedExpr() { m_Domain = SyntaxNodeType::BracketedExpr; }
        vector <unique_ptr<Expr>> Values;
    };

    struct BinaryOperationExpr : public Expr
    {
        //meant to be overriden
        BinaryOperationExpr() { m_Domain = SyntaxNodeType::BinaryOperationExpr; }

        Token m_Operator;


       variant< 
           PrimitiveValue,
           BracketedExpr
       > First, Second;
    };


    struct UnaryOperatorExpr : public Expr
    {
        UnaryOperatorExpr() { m_Domain = SyntaxNodeType::UnaryOperatorExpr; }
        Token m_Operator;

        variant<
            PrimitiveValue,
            BracketedExpr  //could be -1 or -(1 + 2)
        >  Value; 
    };

    struct StatementBlockNode: public StatementNode //used as a scope
    {
        StatementBlockNode() : StatementNode(SyntaxNodeType::StatementBlock) {}
        vector<unique_ptr<StatementNode>> m_CodeBody;
    };

    struct ImportStatement :public StatementNode
    {
        ImportStatement() : StatementNode(SyntaxNodeType::ImportStatement) {}

    };

    //////////////////////////////////////////////
    // Variable assignment and re-assignment
    //////////////////////////////////////////////

    struct VarDeclaration : public Expr
    {
        VarDeclaration() { m_Domain = SyntaxNodeType::VarDeclaration; }
        Token m_VariableName;
        Token m_VariableType;
        Token m_Value;
        int m_ScopeDepth = -1;
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

    struct FieldReassignmentExpr : public Expr
    {
        FieldReassignmentExpr() { m_Domain = SyntaxNodeType::FieldReassignmentExpr; }
    };

    //////////////////////////////////////////////
    // Declarations Involving Multiple Expressions
    //////////////////////////////////////////////

    struct ElseDeclaration : public StatementBlockNode
    {
        ElseDeclaration() { m_Domain = SyntaxNodeType::ElseDeclaration; }
        int m_ScopeDepth = -1;
    };


    struct IfDeclaration : public StatementBlockNode
    {
        IfDeclaration() { m_Domain = SyntaxNodeType::IfDeclaration; }
        vector<Expr> m_Condition; //needs to be a vector since and and or's are a thing
        vector<IfDeclaration> m_ElseIfStatements;
        ElseDeclaration m_ElseStatement;
        int m_ScopeDepth = -1;
    };

    struct FuncDeclaration : public StatementBlockNode
    {
        FuncDeclaration() { m_Domain = SyntaxNodeType::FuncDeclaration; }
        Token m_FuncName;
        Token m_FuncReturnType;

        vector<unique_ptr<Expr>> m_FuncArgs; //tracks all tokens that are relevant for function execution
        vector<unique_ptr<Expr>> m_FuncArgTypes; //arg types should correspond to the same position in m_FuncArgs
    };

    struct MethodDeclaration : public StatementNode
    {
        MethodDeclaration() : StatementNode(SyntaxNodeType::MethodDeclaration) {}
        Token m_AccessLevel; //used for tracking private, public, or protected
        FuncDeclaration m_FunctionDefinition;
    };


    struct WhileLoopDeclaration : public StatementBlockNode
    {
        WhileLoopDeclaration() { m_Domain = SyntaxNodeType::WhileLoopDeclaration; }
        unique_ptr<BracketedExpr> m_Condition; //needs to be a vector since and and or's are a thing
    };

    struct ForLoopDeclaration : public StatementBlockNode
    {
        ForLoopDeclaration() { m_Domain = SyntaxNodeType::ForLoopDeclaration; }
    };


    struct ClassDeclaration : public StatementBlockNode
    {
        ClassDeclaration() { m_Domain = SyntaxNodeType::ClassDeclaration; }

        vector<Token> m_ConstructorArgs; //tracks all tokens that are relevant for the class constructor
        map<string, FieldDeclaration> m_FieldValues;
    };

    //structs are classes without methods, excluding the constructor(s)
    //structs are just used as generic data containers, where operators are defined on it that dictate how this struct interacts with other of its or other types
    struct StructDeclaration : public StatementBlockNode
    {
        vector<Token> m_ConstructorArgs; //tracks all tokens that are relevant for the struct constructor
        StructDeclaration() { m_Domain = SyntaxNodeType::StructDeclaration; }
    };

    struct ScopeDeclaration : public StatementBlockNode
    {
        ScopeDeclaration() { m_Domain = SyntaxNodeType::ScopeDeclaration; }
    };

    struct ListDeclaration : public StatementBlockNode
    {
        ListDeclaration() { m_Domain = SyntaxNodeType::ListDeclaration; }
    };

    struct DictionaryDeclaration : public StatementBlockNode
    {
        DictionaryDeclaration() { m_Domain = SyntaxNodeType::DictionaryDeclaration; }
    };

    //////////////////////////////////////////////
    // Standard Function Types
    //////////////////////////////////////////////
    

    struct PrintFunction : public Expr
    {
        array<unique_ptr<Expr>, 2> m_FuncArgs;

        PrintFunction() { m_Domain = SyntaxNodeType::PrintFunction; }
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

//list of names for all types, used for searching any types being used before they've been defined, or if they've been defined at all Xd
//vector<string> m_ListClassNames;
//vector<string> m_ListStructNames;
//
////these maps hold the var/func name and its corresponding type/return-type, need to be strings since they could be user-defined types
//map<const string, const string> m_MapFuncNames; //can't recast a functions return type though
//map<const string, string> m_MapVariableNames; //types can be recast for variables