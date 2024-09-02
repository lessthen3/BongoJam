#pragma once

#include <string>
#include <vector>

#include "Lexer.h"

using namespace std;

namespace BongoJam {

	enum class SyntaxNodeType
	{
		//////////////////// Program Type That Represents a Single Source File ////////////////////

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

		//////////////////// User Defined Names ////////////////////

		ClassName,
		StructName,
		StringName,
		IntName,
		FloatName,
		BoolName,
		FuncName

	};

	//////////////////////////////////////////////
	// Utility Functions
	//////////////////////////////////////////////


	struct StatementNode
	{
		SyntaxNodeType m_Domain;
	};

	struct Expr : public StatementNode
	{
		//used just for typing xdxdxdxdxdxdxdxdxd
		//expressions always evaluate to a value with a type, however the type of that value is dependent on the expression type
		//so we could implement the value var here, however that would require the use of variant's, and i dont really feel like bringing a cannon to a sword fight
	};

	struct StatementBlockNode: public StatementNode
	{
		vector<Expr> m_CodeBody;
	};

	struct StandardFunction : public StatementNode
	{
		SyntaxNodeType m_Domain = SyntaxNodeType::StandardFunction;
		//vector<const Token> m_FuncArgs;
	};

	struct ImportStatement :public StatementNode
	{

	};

	struct Program : public StatementNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::Program;
		vector<StatementNode> m_ProgramBody;

		//list of names for all types
		vector<string> m_ListClassNames;
		vector<string> m_ListStructNames;

		//these maps hold the var/func name and its corresponding type/return-type, need to be strings since they could be user-defined types
		map<const string, const string> m_MapFuncNames; //can't recast a functions return type though
		map<const string, string> m_MapVariableNames; //types can be recast for variables

		Program() {}
	};

	//////////////////////////////////////////////
	// Variable assignment and re-assignment
	//////////////////////////////////////////////

	struct LetDeclaration : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::LetDeclaration;
		Token m_VariableName;
		Token m_VariableType;
		Token m_Value;
		int m_ScopeDepth;

		explicit LetDeclaration(const Token& fp_VariableName, const Token& fp_VariableType, const Token& fp_Value, const int fp_ScopeDepth)
		{
			m_VariableName = fp_VariableName;
			m_VariableType = fp_VariableType;
			m_Value = fp_Value;
			m_ScopeDepth = fp_ScopeDepth;
		}

		explicit LetDeclaration() : m_ScopeDepth(-1) {} //-1 corresponds to an uninitialized let declaration
	};

	struct FieldDeclaration : public LetDeclaration
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::FieldDeclaration;

		Token m_AccessLevel; //used for tracking private, public, or protected

		explicit FieldDeclaration(const Token& fp_VariableType, const Token& fp_Value, const Token& fp_AccessLevel, const int fp_ScopeDepth)
		{
			m_VariableType = fp_VariableType;
			m_Value = fp_Value;
			m_ScopeDepth = fp_ScopeDepth;
			m_AccessLevel = fp_AccessLevel;
		}

		explicit FieldDeclaration() : LetDeclaration() {} //-1 corresponds to an uninitialized let declaration
	};

	struct VariableReassignmentExpr : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::VariableReassignmentExpr;
	};

	struct FieldReassignmentExpr : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::FieldReassignmentExpr;
	};

	//////////////////////////////////////////////
	// Declarations Involving Multiple Expressions
	//////////////////////////////////////////////

	struct IfDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::IfDeclaration;

		vector<Expr> m_Condition; //needs to be a vector since and and or's are a thing
		int m_ScopeDepth;

		explicit IfDeclaration(const vector<Expr>& fp_Condition, const vector<Expr>& fp_Body, const int fp_ScopeDepth)
		{
			m_Condition = fp_Condition;
			m_CodeBody = fp_Body;
			m_ScopeDepth = fp_ScopeDepth;
		}

		explicit IfDeclaration(): m_ScopeDepth(-1) {}
	};

	struct FuncDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::FuncDeclaration;
		Token m_FuncName;
		Token m_FuncReturnType;

		vector<Token> m_FuncArgs; //tracks all tokens that are relevant for function execution
		vector<Token> m_FuncArgTypes; //arg types should correspond to the same position in m_FuncArgs

		explicit FuncDeclaration(const Token& fp_VariableType, const Token& fp_Value, const int fp_ScopeDepth)
		{

		}

		explicit FuncDeclaration() = default; //
	};

	struct MethodDeclaration : public FuncDeclaration
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::MethodDeclaration;
		Token m_AccessLevel; //used for tracking private, public, or protected

		explicit MethodDeclaration(const Token& fp_VariableType, const Token& fp_Value, const Token& fp_AccessLevel)
		{
			m_AccessLevel = fp_AccessLevel;
		}

		explicit MethodDeclaration() : FuncDeclaration() {} //-1 corresponds to an uninitialized let declaration
	};

	struct WhileLoopDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::WhileLoopDeclaration;
		vector<Expr> m_Condition; //needs to be a vector since and and or's are a thing
	};

	struct ForLoopDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ForLoopDeclaration;

	};

	struct ClassDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ClassDeclaration;

		vector<Token> m_ConstructorArgs; //tracks all tokens that are relevant for the class constructor
		map<string, FieldDeclaration> m_FieldValues;
	};

	//structs are classes without methods, excluding the constructor(s)
	//structs are just used as generic data containers, where operators are defined on it that dictate how this struct interacts with other of its or other types
	struct StructDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::StructDeclaration;
		vector<Token> m_ConstructorArgs; //tracks all tokens that are relevant for the struct constructor
	};

	struct ScopeDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ScopeDeclaration;

	};

	struct ListDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ListDeclaration;

	};

	struct DictionaryDeclaration : public StatementBlockNode
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::DictionaryDeclaration;

	};

	//////////////////////////////////////////////
	// Standard Function Types
	//////////////////////////////////////////////
	
	struct PrintFunction : public StandardFunction
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::PrintFunction;

		Token m_PrintValue;

	};

	struct InputFunction : public StandardFunction
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::InputFunction;
	};

	struct ClockFunction : public StandardFunction
	{

	};

	struct RoundDown : public StandardFunction
	{

	};

	struct RoundUp : public StandardFunction
	{

	};

	struct SquareRoot : public StandardFunction
	{

	};

	struct Power : public StandardFunction
	{

	};

	struct Sin : public StandardFunction
	{

	};

	struct Cos : public StandardFunction
	{

	};

	struct Tan : public StandardFunction
	{

	};

	struct Sinh : public StandardFunction
	{

	};

	struct Cosh : public StandardFunction
	{

	};

	struct Tanh : public StandardFunction
	{

	};

	struct ArcSin : public StandardFunction
	{

	};

	struct ArcCos : public StandardFunction
	{

	};

	struct ArcTan : public StandardFunction
	{

	};

	struct Exponential : public StandardFunction
	{

	};

	//////////////////////////////////////////////
	// Expression Definitions
	//////////////////////////////////////////////

	struct BinaryExpr: public Expr
	{
		//meant to be overriden
		const SyntaxNodeType m_Domain = SyntaxNodeType::BinaryExpr;
		Token m_Operator;

		Expr m_First; //used for ordering of non commutative operations (non-abelian)
		Expr m_Second;

		explicit BinaryExpr(const Expr& fp_First, const Token& fp_Operator, const Expr& fp_Second)
		{
			m_First = fp_First;
			m_Second = fp_Second;
			m_Operator = fp_Operator;
		}

		explicit BinaryExpr() {}
	};

	struct UnaryExpr : public Expr
	{

	};

	//////////////////////////////////////////////
	// Numeric Expressions
	//////////////////////////////////////////////

	struct AdditionExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::AdditionExpr;
	};

	struct SubtractionExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::SubtractionExpr;
	};

	struct MultiplicationExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::MultiplicationExpr;
	};

	struct DivisionExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::DivisionExpr;
	};

	struct ModuloExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ModuloExpr;
	};

	struct PlusEqualsExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::PlusEqualsExpr;
	};

	struct MinusEqualsExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::MinusEqualsExpr;
	};

	struct MultEqualsExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::MultEqualsExpr;
	};

	struct DivEqualsExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::DivEqualsExpr;
	};

	struct ModuloEqualsExpr : public BinaryExpr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ModuloEqualsExpr;
	};

	struct BracketedExpr : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::BracketedExpr;
		vector<BinaryExpr> m_BinaryExpressions;
		vector<UnaryExpr> m_UnaryExpressions;
	};

	struct MethodCallExpr : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::MethodCallExpr;
		Token m_MethodName;
		Token m_ReturnType;
		vector<Token> m_MethodArgs;
	};

	struct FunctionCallExpr : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::FunctionCallExpr;
		Token m_FuncName;
		Token m_ReturnType;
		vector<Token> m_FuncArgs;
	};

	struct ClassCallExpr : public Expr //used for: "let x->myClass = new myClass();"
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::ClassCallExpr;

		bool m_IsHeapAllocated;

		explicit ClassCallExpr(const bool fp_IsHeapAllocated = false)
		{
			m_IsHeapAllocated = fp_IsHeapAllocated;
		}

		explicit ClassCallExpr(): m_IsHeapAllocated(false) {}
	};

	struct StructCallExpr : public Expr //used for: "let x->myStruct = new myStruct();"
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::StructCallExpr;

		bool m_IsHeapAllocated;

		explicit StructCallExpr(const bool fp_IsHeapAllocated = false)
		{
			m_IsHeapAllocated = fp_IsHeapAllocated;
		}

		explicit StructCallExpr() : m_IsHeapAllocated(false) {}
	};

	//////////////////////////////////////////////
	// Boolean Expressions
	//////////////////////////////////////////////

	struct GreaterThanExpr : public Expr
	{

	};

	struct GreaterThanOrEqualsExpr : public Expr
	{

	};

	struct LesserThanExpr : public Expr
	{

	};

	struct LesserThanOrEqualsExpr : public Expr
	{

	};

	struct StrictlyEqualsExpr : public Expr
	{

	};

	struct AndExpr : public Expr
	{

	};

	struct OrExpr : public Expr
	{

	};

	struct NotExpr : public Expr
	{

	};

	//////////////////////////////////////////////
	// Literal Values of Corresponding Types
	//////////////////////////////////////////////

	struct IntLiteral : public Expr //all ints are 32-bits, fuck you the register size is big enough
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::IntLiteral;
		int32_t m_Value;

		explicit IntLiteral(int32_t fp_Value)
		{
			m_Value = fp_Value;
			5 + 5; false; false; false; true;
		}
	};

	struct FloatLiteral : public Expr //we treat all numbers as floats until we get to the interpreter, where it will re-cast the number as an int if needed
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::FloatLiteral;
		float m_Value;

		explicit FloatLiteral(float fp_Value)
		{
			m_Value = fp_Value;
		}
	};

	struct StringLiteral : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::StringLiteral;
		string m_Value;

		explicit StringLiteral(const string& fp_Value) //we're just using the default cpp string implementation, I don't see why not
		{
			m_Value = fp_Value;
		}
	};

	struct NullLiteral : public Expr
	{
		const SyntaxNodeType m_Domain = SyntaxNodeType::NullLiteral;
		const nullptr_t null = nullptr;
	};
}