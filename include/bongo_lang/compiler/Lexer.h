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

#include <algorithm>
#include <cctype>
#include <cassert>

#include "../Logger.h"

#include <numbers>

#define BONGO_PI std::numbers::pi
#define BONGO_EULERS_NUMBER std::numbers::e


namespace BongoJam {

    //////////////////////////////////////////////
    // Token and Token-type Definition
    //////////////////////////////////////////////

    enum class TokenType
    {
        //////////////////// GOATS ////////////////////

        UnsignedIntNumber, //idk
        IntNumber,
        FloatNumber,
        DoubleNumber,
        CharLiteral, //char value idk how it differs from an int but idk just here in case uwu
        StringLiteral, //just plain text
        UserIdentifier, //var or class names, function calls, function names, class constructors

        FormattedStringLiteralStart, // f"uwuuwu its {name} uwuwuwuwu >O<"
        FormattedStringLiteralEnd,
        FormattedStringInsert,

        Include,
        NameSpace,

        //////////////////// Operators ////////////////////

        Equals,
        StrictlyEquals,
        DoesNotEquals,

        GreaterThan,
        LesserThan,
        GreaterThanOrEqual,
        LesserThanOrEqual,

        AdditionOperator,
        NegativeOperator, //don't need subtraction

        MultiplicationOperator,
        DivisionOperator,

        ModulusOperator,

        PlusEqualsOperator,
        MinusEqualsOperator,
        MultEqualsOperator,
        DivEqualsOperator,

        MinusMinusOperator,
        PlusPlusOperator,

        ModuloEqualsOperator,

        BitshiftRightOperator,
        BitshiftLeftOperator,

        //no Bitand since & is already being used as a token generally as ampersand, not lexing '|' or '~' atm so ye need those
        BitOrOperator,
        BitNotOperator,
        BitXorOperator,

        BitshiftRightEquals,
        BitshiftLeftEquals,

        BitAndEquals,
        BitOrEquals,
        BitXorEquals,

        //////////////////// Bracket Types ////////////////////

        OpenParen,
        CloseParen,

        OpenBracket,
        CloseBracket,

        OpenSquareBracket,
        CloseSquareBracket,

        //////////////////// Symbols ////////////////////

        DoubleDot,
        SemiDot,
        Dot,
        Comma,

        DollarSign,
        AtSign,
        QuestionMark,
        QuotationMark,
        HashTag,
        Ampersand,

        //////////////////// Declaratives ////////////////////

        Var,
        Func,
        Return,
        Class,
        Struct,
        Enum,
        Interface,

        //////////////////// Conditional Logic ////////////////////

        If,
        Elif,
        Else,
        For,
        While,
        Do,
        Match,
        Break,
        Continue,
        Try,
        Catch,
        Scope,

        //////////////////// Boolean Operators ////////////////////

        And,
        Or,
        Not,
        Is,

        //////////////////// Modifiers ////////////////////

        Const,
        Static,
        Public,
        Protected,
        Private, //used for tracking later on in the parser and interpreter
        As,
        In,
        Extends,
        Single,
        Template, //used for defining abstract class'

        Parent,
        This,
        Big, //used as the enlargement modifier for container sizing of floats and ints

        New,
        Delete,

        ThreadSafe,

        //////////////////// Error Handling ////////////////////

        Panic,
        StaticAssert,
        Assert,

        //////////////////// Types ////////////////////

        TypeArrow,
        Void,
        Exception,

        UnsignedInt,
        Int,
        Float,
        Double,
        Bool,
        True,
        False,
        String, //string type decl
        Char,

        List,
        Dictionary,

        Thread,

        Event,

        Leash, //any var can have a manager, but only one at a time. you can have managers borrow clients from other managers temporarily, but the lifetime must not exceed the length of the original manager because ultimately the lifetime of the client is still tied to the manager
        Watcher,

        Move,
        Bounce,

        Vector2,
        Vector3,
        Vector4,
        Vector,

        Mat2,
        Mat3,
        Mat4,
        Mat,

        //////////////////// Included Functions ////////////////////

        ToString,
        SizeOf,
        Print,
        Input,

        Clock,
        TypeOf,
        UpCast,
        DownCast,
        StaticCast,
        Length,

        Floor,
        Ceiling,

        SquareRoot,
        Power,

        Sin,
        Cos,
        Tan,

        ArcSin,
        ArcCos,
        ArcTan,

        Sinh,
        Cosh,
        Tanh,

        Exp,
        NaturalLog,
        Factorial,

        Colourize,

        NO_TOKEN_VALUE,
        ENDF
    };
    
    struct Token
    {
        string m_Value;
        TokenType m_Type;
        size_t m_SourceCodeLineNumber;

        explicit Token(const string& fp_Value, const TokenType fp_Type, const size_t fp_SourceCodeLineNumber)
        {
            m_Value = fp_Value;
            m_Type = fp_Type;
            m_SourceCodeLineNumber = fp_SourceCodeLineNumber;
        }

        explicit Token(const char& fp_Value, const TokenType fp_Type, const size_t fp_SourceCodeLineNumber)
        {
            m_Value = fp_Value;
            m_Type = fp_Type;
            m_SourceCodeLineNumber = fp_SourceCodeLineNumber;
        }

        explicit Token(): m_Value(""), m_Type(TokenType::NO_TOKEN_VALUE), m_SourceCodeLineNumber(-1) {}
    };

    //////////////////////////////////////////////
    // Keywords Definition
    //////////////////////////////////////////////

    const map<string, TokenType> KEYWORDS =
    {
        //////////////////// GOAT ////////////////////

        {"include", TokenType::Include}, //PLEASE I LOVE USING OTHER PEOPLE'S CODE
        {"namespace", TokenType::NameSpace}, //used for namespaces, name identifier {}

        //////////////////// Declaratives ////////////////////

        {"var", TokenType::Var},
        {"func", TokenType::Func},
        {"return", TokenType::Return},
        {"class", TokenType::Class},
        {"struct", TokenType::Struct},
        {"enum", TokenType::Enum},
        {"interface", TokenType::Interface},

        //////////////////// Conditional Logic ////////////////////

        {"if", TokenType::If},
        {"elif", TokenType::Elif},
        {"else", TokenType::Else},
        {"for", TokenType::For},
        {"while", TokenType::While},
        {"do", TokenType::Do}, //do and else statments are compatible with while loops
        {"match", TokenType::Match}, //switch statement
        {"break", TokenType::Break},
        {"continue", TokenType::Continue},
        {"try", TokenType::Try},
        {"catch", TokenType::Catch},
        {"scope", TokenType::Scope},

        //////////////////// Boolean Operators ////////////////////

        {"and", TokenType::And},
        {"or", TokenType::Or},
        {"not", TokenType::Not},
        {"is", TokenType::Is}, // used for type checking

        //////////////////// Modifiers ////////////////////

        {"const", TokenType::Const},
        {"static", TokenType::Static},
        {"protected", TokenType::Protected},
        {"private", TokenType::Private},
        {"as", TokenType::As}, //type cast operator
        {"in", TokenType::In}, //for list/dict searches uwu
        {"extends", TokenType::Extends},
        {"single", TokenType::Single}, //explicit singleton keyword
        {"template", TokenType::Template}, //func's or class' or var's can be templated, this is the replacement for virtual.

        {"parent", TokenType::Parent}, //used as the stand-in for super, because super is a retarded name for the keyword
        {"this", TokenType::This},
        {"big", TokenType::Big}, //used for expanding float or int container size to 64 or 128 bits

        {"threadsafe", TokenType::ThreadSafe}, // used as a stand in for atomic, we copy the value for paralell, try to queue all actions done on it and execute it in a non-sequence breaking order
        //we guess how many copies will be needed, if our guess is wrong we increase it, sequence breaking isnt a big deal, but it is for ppl who expect a consistent behaviour which is me

        //////////////////// Types ////////////////////

        {"void", TokenType::Void},
        {"exception", TokenType::Exception},

        {"int", TokenType::Int},
        {"decimal", TokenType::Float},
        {"bool", TokenType::Bool},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"text", TokenType::String},

        {"List", TokenType::List},
        {"Dictionary", TokenType::Dictionary},

        {"Thread", TokenType::Thread},

        {"event", TokenType::Event},

        {"Vec2", TokenType::Vector2},
        {"Vec3", TokenType::Vector3},
        {"Vec4", TokenType::Vector4},
        {"Vec", TokenType::Vector}, //generic n-length vector

        {"Mat2", TokenType::Mat2},
        {"Mat3", TokenType::Mat3},
        {"Mat4", TokenType::Mat4},
        {"Mat", TokenType::Mat}, //generic nxm matrix
    };

    //////////////////////////////////////////////
    // Colours Map for Easier Checking
    //////////////////////////////////////////////

    const vector<string> ANSI_COLOURS =
    {
        "bk",
        "rd",
        "gn",
        "yw",
        "be",
        "ma",
        "cn",  
        "we", 

        "bbk", 
        "brd", 
        "bgn", 
        "byw",
        "bbe", 
        "bma", 
        "bcn", 
        "bwe"
    };

    [[nodiscard]] char
        ShiftForward(string& fp_Src);

    [[nodiscard]] char
        Peek(const string& fp_Src);

    [[nodiscard]] Token
        LexNumber
        (
            string& fp_Src,
            char& fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber,
            Logger* logger
        );

    [[nodiscard]] string
        LexWord //gets a single alphabetical unit uwu
        (
            string& fp_Src,
            char& fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber
        );

    [[nodiscard]] bool
        LexOperator
        (
            string& fp_Src,
            char& fp_CurrentChar,
            vector<Token>& fp_ProgramTokens,
            size_t& fp_CurrentLineNumber,
            bool& fp_IsCurrentlyInsideComment,
            Logger* logger
        );

    bool
        Tokenize
        (
            string& fp_SourceCode,
            vector<Token>& fp_Tokens,
            Logger* logger
        );
}