#pragma once

#include <stdint.h>

//////////////////////////////////////////////
// Opcodes Lookup for Translation
//////////////////////////////////////////////

enum class OPCODES : uint16_t //these opcodes are flags that indicate to the interpreter what information is going to follow it in the byte stream
{
	NOP = 0x0000,

	//////////////////// Basic Math Operations ////////////////////

	ADD,
	SUB,

	DIV,
	MULT,

	POW,
	SQRT,

	//////////////////// Memory Operations ////////////////////

	LOAD_VAL,
	LOAD_CONST,

	STACK_ALLOC, //stack allocates a var
	STACK_FREE, //dereferences stack alloc'd var in interpreter
	MOVE, //move() semantic
	COPY, //used for copying vars
	HEAP_ALLOC, //used for heap allocations
	HEAP_FREE, //delete baby

	//////////////////// Control Flow Operations ////////////////////

	JUMP,
	JUMP_IF,

	CMP,

	//////////////////// Extern Call for C/C++ ////////////////////

	NATIVE_CALL,

	//////////////////// OS Access Codes ////////////////////

	STDOUT,
	STDERR,

	CLOCK_START,
	CLOCK_END,

	SLEEP,
	THREAD,

	INPUT,

	//////////////////// Primitive Types ////////////////////

	INT_VALUE,
	FLOAT_VALUE,
	BOOL_VALUE, //false or true follows, 0 = false, 1 = true as always

	CHAR_VALUE,
	STRING_VALUE, //string literal value always follows, pattern is: str size in bytes -> encoded utf-8 str

	VOID_VALUE,

	//////////////////// Unsure ////////////////////

	COLOURIZE,
	STRING_LITERAL, //used for detecting constant strings in the byte code for translation during runtime
	EXCEPTION,
	ROUND_UP,
	ROUND_DOWN,

	POWER,
	EXP,

	SIN,
	COS,
	SINH,
	COSH,
	ARCSIN,
	ARCCOS,

	LOG,
	FACTORIAL,

	//////////////////// Built-in Class Types ////////////////////

	VEC2,
	VEC3,
	VEC4,

	MAT2,
	MAT3,
	MAT4,
	MAT,

	//////////////////// Boolean Comparison Operations ////////////////////

	LOGICAL_AND,
	LOGICAL_OR,
	LOGICAL_NOT,

	LINE_NUMBER, //Used for tracking the exact line of code that threw a runtime error

	VARIABLE_REASSIGNMENT,

	//////////////////// Function/Method Operations ////////////////////

	FUNCTION_DEFINITION,
	METHOD_DEFINITION,

	FUCTION_CALL,
	METHOD_CALL,

	FUNCTION_RETURN,

	//////////////////// Class/Struct Operations ////////////////////

	CLASS_DEFINITION,
	STRUCT_DEFINITION,

	CLASS_CONSTRUCTOR,
	STRUCT_CONSTRUCTOR,

	HALT = 0xFFFF
};