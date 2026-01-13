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

#define BONGO_COMPILER_VERSION "0.0.1"

///BongoJam
#include "Parser.h"
#include "../Opcodes.h"

#include "../Serializer.h"

namespace BongoJam {

    enum BongoCompilerFlags : uint64_t
    {
        DEFAULT = 0,

        DEBUG = 1U << 0,
        RELEASE = 1U << 1,

        BUILD_STATIC_LIBRARY = 1U << 2,
        BUILD_DYNAMIC_LIBRARY = 1U << 3,
        BUILD_EXECUTABLE = 1U << 5
    };

    enum SymbolKind : uint8_t
    { 
        INVALID_SYMBOL = 0, 
        Variable = 1 << 0, 
        Field = 1 << 1, 
        Function = 1 << 2, 
        Method = 1 << 3, 
        Struct = 1 << 4,
        Class = 1 << 5 
    };

    struct Symbol
    {
        string Name;

        TokenType Type = TokenType::NO_TOKEN_VALUE; // for type checking

        uint8_t Flags = ModifierFlags::NONE;
        uint8_t Kind = SymbolKind::INVALID_SYMBOL; // FUNCTION, STRUCT, CLASS, GLOBAL_VAR

        size_t Slot = 0;
        size_t OffsetInBytecode = 0; // Offset based off the compilation unit the compilationunit base offset will be recorded by the linker for resolving symbols

        bool IsResolved = false;

        Symbol(const string& fp_Name, SymbolKind fp_Kind, TokenType fp_Type, const size_t fp_Offset, const uint8_t fp_SymFlags)
            : Name(fp_Name), Kind(fp_Kind), Type(fp_Type), OffsetInBytecode(fp_Offset), Flags(fp_SymFlags) {}

        Symbol() = default;
    };

    struct CompilationUnit
    {
        string ScriptPath; //compilation units rae generated per script so # of scripts = # of compilation units

        vector<uint8_t> CompiledByteCode; //bytecode
        vector<SSAInstruction> SSA_IR; //

        unordered_map<string, Symbol> SymbolTable; // symbol name : symbol information
        unordered_map<string, Symbol> UnresolvedSymbolTable; //hf linker

        vector<string> Includes;

        //debugname table quesiton mark???_????
    };

    struct BongoScriptUnit
    {
        filesystem::path FilePath;
        unique_ptr<CompilationUnit> CompiledUnit = make_unique<CompilationUnit>();
    };
}

namespace BongoJam {

    [[nodiscard]] constexpr string
        CreateColouredText
        (
            const string& fp_SampleText,
            const string& fp_DesiredColour
        )
    {
        string f_NormalizedColour = fp_DesiredColour;

        //////////////////// Regular Colours ////////////////////

        if (f_NormalizedColour == "bk")
        {
            return "\x1B[30m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "rd")
        {
            return "\x1B[31m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "gn")
        {
            return "\x1B[32m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "yw")
        {
            return "\x1B[33m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "be")
        {
            return "\x1B[34m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "ma")
        {
            return "\x1B[35m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "cn")
        {
            return "\x1B[36m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "we")
        {
            return "\x1B[37m" + fp_SampleText + "\033[0m";
        }

        //////////////////// Bright Colours ////////////////////

        if (f_NormalizedColour == "bbk")
        {
            return "\x1B[90m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "brd")
        {
            return "\x1B[91m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "bgn")
        {
            return "\x1B[92m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "byw")
        {
            return "\x1B[93m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "bbe")
        {
            return "\x1B[94m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "bma")
        {
            return "\x1B[95m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "bcn")
        {
            return "\x1B[96m" + fp_SampleText + "\033[0m";
        }
        else if (f_NormalizedColour == "bwe")
        {
            return "\x1B[97m" + fp_SampleText + "\033[0m";
        }
        //////////////////// Just Return the Input Text Unaltered Otherwise ////////////////////
        else
        {
            return fp_SampleText;
        }
    }

    struct BongoCompiler
    {
        ~BongoCompiler() = default;

        BongoCompiler(const string& fp_CompilerName);

    public:
        shared_ptr<Logger> compiler_logger = nullptr; //shared for now cause idk how else work

    ///////////////////////////////////////////////////////////// PRIVATE /////////////////////////////////////////////////////////////
    private:
        unique_ptr<Parser> pm_BongoParser = nullptr; //needa make this a class since that's the only way cpp will let me do mutual recursion for some reason lmao

        size_t pm_CompiledThreadID = 0;
        
        string pm_CompilerName = "NO_COMPILER_NAME";
        uint8_t pm_NextAvailableStackSlot = 0;

    private:
        ////////////////////////////////////////////// Utility Functions //////////////////////////////////////////////
        
        unique_ptr<StatementNode>
            ShiftForward(vector<unique_ptr<StatementNode>>& fp_ProgramBody);

        bool
            TryToResolveSymbol(const string& fp_SymbolName, CompilationUnit* fp_CompilationUnit);

        bool
            ReadFileIntoString
            (
                string* fp_SourceCode,
                const string& fp_ScriptFilePath
            )
            const;

        size_t
            GetCurrentByteOffset(CompilationUnit* fp_CompilationUnit)
        {
            return fp_CompilationUnit->CompiledByteCode.size() - 1; //-1 because its accessing a vector index so it starts at 0 uwu
        }

    public:
        ////////////////////////////////////////////// MAIN COMPILING FUNCTION //////////////////////////////////////////////

        int
            CompileUnit
            (
                const string& fp_DesiredBongoScriptFilePath,
                CompilationUnit* fp_CompilationUnit,
                const bool fp_IsDebug = false
            );

        bool
            CompilePrintFunction
            (
                FunctionCallExpr* fp_PrintFunction,
                CompilationUnit* fp_CompilationUnit
            );


        bool
            CompileInputFunction
            (
                FunctionCallExpr* fp_PrintFunction,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileDeclaredFunction
            (
                FuncDeclaration* fp_FuncDeclaration,
                CompilationUnit* fp_CompilationUnit,
                const string& fp_NameSpace = ""
            );

        bool
            CompileDeclaredClass
            (
                ClassDeclaration* fp_FuncDeclaration,
                CompilationUnit* fp_CompilationUnit,
                const string& fp_NameSpace = ""
            );

        bool
            CompileDeclaredStruct
            (
                StructDeclaration* fp_FuncDeclaration,
                CompilationUnit* fp_CompilationUnit,
                const string& fp_NameSpace = ""
            );

        bool
            CompileVarDeclaration
            (
                VarDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit,
                const string& fp_NameSpace = ""
            );

        bool
            CompileFieldDeclaration
            (
                VarDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileRegularExpr
            (
                Expr* fp_Expression,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileFuncCall
            (
                FunctionCallExpr* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileIfStatement
            (
                IfDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileWhileLoop
            (
                WhileLoopDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileForLoop
            (
                ForLoopDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileVarReassignment 
            (
                VariableReassignmentExpr* fp_Expression,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileUserIdentifier
            (
                Expr* fp_SymbolExpr,
                CompilationUnit* fp_CompilationUnit
            );
    };
}