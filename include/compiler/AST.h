/*******************************************************************
 *                                        BongoJam Script v0.3                                        
 *                           Created by Ranyodh Mandur - � 2024                            
 *                                                                                                                  
 *                         Licensed under the MIT License (MIT).                           
 *                  For more details, see the LICENSE file or visit:                     
 *                        https://opensource.org/licenses/MIT                               
 *                                                                                                                  
 *  BongoJam is an open-source scripting language compiler and interpreter 
 *              primarily intended for embedding within game engines.               
********************************************************************/
#pragma once

#include "Lexer.h"

using namespace std;

namespace BongoJam {

	enum class SyntaxNodeType
	{
		//////////////////// Program Type That Represents All Source Files Compiled From Local ////////////////////

		Program,

		//////////////////// Literal Values of Types ////////////////////

		IntLiteral,
		FloatLiteral,
		StringLiteral,
		NullLiteral,

		FalseLiteral,
		TrueLiteral,

		//////////////////// User Declarations ////////////////////

		LetDeclaration,
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

		//////////////////// Built-in Class Types ////////////////////

		ListDeclaration,
		DictionaryDeclaration,

		//////////////////// Expressions ////////////////////

		BinaryExpr,
		UnaryExpr,

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
		InputFunction,

		ClockFunction,
		TypeOfFunction,

		RoundDown,
		RoundUp,

		SquareRootFunction,
		PowerFunction,

		SinFunction,
		CosFunction,
		TanFunction,

		SinhFunction,
		CoshFunction,
		TanhFunction,

		ArcSinFunction,
		ArcCosFunction,
		ArcTanFunction,

		ExpFunction,
		LogFunction,
		FactorialFunction,

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

	//////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////


	struct StatementNode
	{
		SyntaxNodeType m_Domain = SyntaxNodeType::None;
		virtual ~StatementNode() {}
		//virtual llvm::Value* codegen() = 0;
	};

	struct Expr : public StatementNode
	{
		//used just for typing xdxdxdxdxdxdxdxdxd
		//expressions always evaluate to a value with a type, however the type of that value is dependent on the expression type
		//so we could implement the value var here, however that would require the use of variant's, and i dont really feel like bringing a cannon to a sword fight
	};

	struct StatementBlockNode: public StatementNode
	{
		vector<unique_ptr<StatementNode>> m_CodeBody;
	};

	struct StandardFunction : public StatementNode
	{
		vector< //outer list is (arg1, arg2)
			vector< //inner list is if arg1 = funcCall1() + funcCall2() - 3 / "hello world" + 3.14156969
				unique_ptr<Expr>
		>> m_FuncArgs;

		StandardFunction(): StatementNode() { m_Domain = SyntaxNodeType::StandardFunction; }
	};

	struct ImportStatement :public StatementNode
	{
		ImportStatement() { m_Domain = SyntaxNodeType::ImportStatement; }

	};

	//////////////////////////////////////////////
	// Variable assignment and re-assignment
	//////////////////////////////////////////////

	struct LetDeclaration : public Expr
	{
		LetDeclaration() { m_Domain = SyntaxNodeType::LetDeclaration; }
		Token m_VariableName;
		Token m_VariableType;
		Token m_Value;
		int m_ScopeDepth = -1;
	};

	struct FieldDeclaration : public LetDeclaration
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

	struct ElseDeclaration : StatementBlockNode
	{
		ElseDeclaration() { m_Domain = SyntaxNodeType::ElseDeclaration; }
		int m_ScopeDepth = -1;
	};

	struct IfDeclaration : public StatementBlockNode
	{
		IfDeclaration() { m_Domain = SyntaxNodeType::IfDeclaration; }
		vector<Expr> m_Condition; //needs to be a vector since and and or's are a thing
		vector<unique_ptr<IfDeclaration>> m_ElseIfStatements;
		unique_ptr<ElseDeclaration> m_ElseStatement;
		int m_ScopeDepth = -1;
	};

	struct FuncDeclaration : public StatementBlockNode
	{
		FuncDeclaration() { m_Domain = SyntaxNodeType::FuncDeclaration; }
		Token m_FuncName;
		Token m_FuncReturnType;

		vector<Token> m_FuncArgs; //tracks all tokens that are relevant for function execution
		vector<Token> m_FuncArgTypes; //arg types should correspond to the same position in m_FuncArgs
	};

	struct MethodDeclaration : public StatementNode
	{
		MethodDeclaration() { m_Domain = SyntaxNodeType::MethodDeclaration; }
		Token m_AccessLevel; //used for tracking private, public, or protected
		FuncDeclaration m_FunctionDefinition;
	};

	struct WhileLoopDeclaration : public StatementBlockNode
	{
		WhileLoopDeclaration() { m_Domain = SyntaxNodeType::WhileLoopDeclaration; }
		vector<Expr> m_Condition; //needs to be a vector since and and or's are a thing
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
	
	struct PrintFunction : public StandardFunction
	{
		PrintFunction() 
		{  
			m_Domain = SyntaxNodeType::PrintFunction;
			m_FuncArgs.push_back(vector<unique_ptr<Expr>>()); //reserve doesnt work here uwu
			m_FuncArgs.push_back(vector<unique_ptr<Expr>>());
		}
	};

	struct InputFunction : public StandardFunction
	{
		InputFunction() { m_Domain = SyntaxNodeType::InputFunction; }
	};

	struct ClockFunction : public StandardFunction
	{
		ClockFunction() { m_Domain = SyntaxNodeType::ClockFunction; }
	};

	struct RoundDown : public StandardFunction
	{
		RoundDown() { m_Domain = SyntaxNodeType::RoundDown; }
	};

	struct RoundUp : public StandardFunction
	{
		RoundUp() { m_Domain = SyntaxNodeType::RoundUp; }
	};

	struct SquareRoot : public StandardFunction
	{
		SquareRoot() { m_Domain = SyntaxNodeType::SquareRootFunction; }
	};

	struct Power : public StandardFunction
	{
		Power() { m_Domain = SyntaxNodeType::PowerFunction; }
	};

	struct Sin : public StandardFunction
	{
		Sin() { m_Domain = SyntaxNodeType::SinFunction; }
	};

	struct Cos : public StandardFunction
	{
		Cos() { m_Domain = SyntaxNodeType::CosFunction; }
	};

	struct Tan : public StandardFunction
	{
		Tan() { m_Domain = SyntaxNodeType::TanFunction; }
	};

	struct Sinh : public StandardFunction
	{
		Sinh() { m_Domain = SyntaxNodeType::SinhFunction; }
	};

	struct Cosh : public StandardFunction
	{
		Cosh() { m_Domain = SyntaxNodeType::CoshFunction; }
	};

	struct Tanh : public StandardFunction
	{
		Tanh() { m_Domain = SyntaxNodeType::TanhFunction; }
	};

	struct ArcSin : public StandardFunction
	{
		ArcSin() { m_Domain = SyntaxNodeType::ArcSinFunction; }
	};

	struct ArcCos : public StandardFunction
	{
		ArcCos() { m_Domain = SyntaxNodeType::ArcCosFunction; }
	};

	struct ArcTan : public StandardFunction
	{
		ArcTan() { m_Domain = SyntaxNodeType::ArcTanFunction; }
	};

	struct Exponential : public StandardFunction
	{
		Exponential() { m_Domain = SyntaxNodeType::ExpFunction; }
	};

	struct Log : public StandardFunction
	{
		Log() { m_Domain = SyntaxNodeType::LogFunction; }
	};

	struct Factorial : public StandardFunction
	{
		Factorial() { m_Domain = SyntaxNodeType::FactorialFunction; }
	};

	//////////////////////////////////////////////
	// Expression Definitions
	//////////////////////////////////////////////

	struct BinaryExpr: public Expr
	{
		//meant to be overriden
		BinaryExpr() { m_Domain = SyntaxNodeType::BinaryExpr; }
		Token m_Operator;

		Expr m_First; //used for ordering of non commutative operations (non-abelian)
		Expr m_Second;
	};

	struct UnaryExpr : public Expr
	{
		UnaryExpr() { m_Domain = SyntaxNodeType::UnaryExpr; }
		Token m_Operator;

		Expr m_Only;
	};

	//////////////////////////////////////////////
	// Numeric Expressions
	//////////////////////////////////////////////

	struct AdditionExpr : public BinaryExpr
	{
		AdditionExpr() { m_Domain = SyntaxNodeType::AdditionExpr; }
	};

	struct SubtractionExpr : public BinaryExpr
	{
		SubtractionExpr() { m_Domain = SyntaxNodeType::SubtractionExpr; }
	};

	struct MultiplicationExpr : public BinaryExpr
	{
		MultiplicationExpr() { m_Domain = SyntaxNodeType::MultiplicationExpr; }
	};

	struct DivisionExpr : public BinaryExpr
	{
		DivisionExpr() { m_Domain = SyntaxNodeType::DivisionExpr; }
	};

	struct ModuloExpr : public BinaryExpr
	{
		ModuloExpr() { m_Domain = SyntaxNodeType::ModuloExpr; }
	};

	struct PlusEqualsExpr : public BinaryExpr
	{
		PlusEqualsExpr() { m_Domain = SyntaxNodeType::PlusEqualsExpr; }
	};

	struct MinusEqualsExpr : public BinaryExpr
	{
		MinusEqualsExpr() { m_Domain = SyntaxNodeType::MinusEqualsExpr; }
	};

	struct MultEqualsExpr : public BinaryExpr
	{
		MultEqualsExpr() { m_Domain = SyntaxNodeType::MultEqualsExpr; }
	};

	struct DivEqualsExpr : public BinaryExpr
	{
		DivEqualsExpr() { m_Domain = SyntaxNodeType::DivEqualsExpr; }
	};

	struct ModuloEqualsExpr : public BinaryExpr
	{
		ModuloEqualsExpr() { m_Domain = SyntaxNodeType::ModuloEqualsExpr; }
	};

	struct BracketedExpr : public Expr
	{
		BracketedExpr() { m_Domain = SyntaxNodeType::ExpFunction; }
		vector<BinaryExpr> m_BinaryExpressions;
		vector<UnaryExpr> m_UnaryExpressions;
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

	struct StructCallExpr : public Expr //used for: "let x->myStruct = new myStruct();"
	{
		StructCallExpr() { m_Domain = SyntaxNodeType::StructCallExpr; }

		bool m_IsHeapAllocated = false;
	};

	//////////////////////////////////////////////
	// Boolean Expressions
	//////////////////////////////////////////////

	struct GreaterThanExpr : public Expr
	{
		GreaterThanExpr() { m_Domain = SyntaxNodeType::GreaterThanExpr; }

	};

	struct GreaterThanOrEqualsExpr : public Expr
	{
		GreaterThanOrEqualsExpr() { m_Domain = SyntaxNodeType::GreaterThanOrEqualsExpr; }

	};

	struct LesserThanExpr : public Expr
	{
		LesserThanExpr() { m_Domain = SyntaxNodeType::LesserThanExpr; }

	};

	struct LesserThanOrEqualsExpr : public Expr
	{
		LesserThanOrEqualsExpr() { m_Domain = SyntaxNodeType::LesserThanOrEqualsExpr; }

	};

	struct StrictlyEqualsExpr : public Expr
	{
		StrictlyEqualsExpr() { m_Domain = SyntaxNodeType::StrictlyEqualsExpr; }

	};

	struct AndExpr : public Expr
	{
		AndExpr() { m_Domain = SyntaxNodeType::AndExpr; }

	};

	struct OrExpr : public Expr
	{
		OrExpr() { m_Domain = SyntaxNodeType::OrExpr; }

	};

	struct NotExpr : public Expr
	{
		NotExpr() { m_Domain = SyntaxNodeType::NotExpr; }

	};

	//////////////////////////////////////////////
	// Literal Values of Corresponding Types
	//////////////////////////////////////////////

	struct IntLiteral : public Expr //all ints are 32-bits, fuck you the register size is big enough
	{
		int32_t m_Value;

		explicit IntLiteral(const int32_t fp_Value) 
		{
			m_Domain = SyntaxNodeType::IntLiteral; 
			m_Value = fp_Value;
		}
	};

	struct FloatLiteral : public Expr //we treat all numbers as floats until we get to the interpreter, where it will re-cast the number as an int if needed
	{
		float m_Value;

		explicit FloatLiteral(const float fp_Value) 
		{ 
			m_Domain = SyntaxNodeType::FloatLiteral; 
			m_Value = fp_Value;
		}
	};

	struct StringLiteral : public Expr
	{
		Token m_StringValue;

		//we're just using the default cpp string implementation, I don't see why not
		explicit StringLiteral(const Token& fp_StringValue) 
		{ 
			m_Domain = SyntaxNodeType::StringLiteral;
			m_StringValue = fp_StringValue;
		}
	};

	struct NullLiteral : public Expr
	{
		explicit NullLiteral() //we dont need a value since null is always just null
		{
			m_Domain = SyntaxNodeType::NullLiteral;
		}
	};

	//////////////////////////////////////////////
	// Sub-Statements
	//////////////////////////////////////////////

	struct ReturnSubStatement : public StatementNode
	{
		ReturnSubStatement() { m_Domain = SyntaxNodeType::ReturnSubStatement; }

		vector<BracketedExpr> m_ReturnExpression; //contains the order in which the expression should be done for a particular return statement
	};

	struct BreakSubStatement :public StatementNode
	{
		BreakSubStatement() { m_Domain = SyntaxNodeType::BreakSubStatement; }
	};


	struct Program : public StatementNode
	{
		Program() { m_Domain = SyntaxNodeType::Program; }

		vector<ClassDeclaration> m_ProgramClasses; //contains all statements held inside the main func
		vector<StructDeclaration> m_ProgramStructs;

		//first function should be the very first function, defined in the very top level of the import tree
		vector <unique_ptr<FuncDeclaration>> m_ProgramFunctions; //contains all defined functions inside the script

		//list of names for all types, used for searching any types being used before they've been defined, or if they've been defined at all Xd
		vector<string> m_ListClassNames;
		vector<string> m_ListStructNames;

		//these maps hold the var/func name and its corresponding type/return-type, need to be strings since they could be user-defined types
		map<const string, const string> m_MapFuncNames; //can't recast a functions return type though
		map<const string, string> m_MapVariableNames; //types can be recast for variables
	};

}