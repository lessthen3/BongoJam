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

    enum SymbolFlag : uint8_t
    {
        NO_FLAGS = 0,

        IS_STATIC = 1 << 0,
        IS_CONST = 1 << 1,
        IS_SINGLE = 1 << 2,

        IS_HEAP = 1 << 3,

        PRIVATE_SYM = 1 << 4,
        PROTECTED_SYM = 1 << 5,
        PUBLIC_SYM = 1 << 6,

        IS_RESOLVED = 1 << 7
    };

    enum class SymbolKind { INVALID, Variable, Field, Function, Method, Struct, Class };

    struct Symbol
    {
        string Name;

        SymbolKind Kind = SymbolKind::INVALID; // FUNCTION, STRUCT, CLASS, GLOBAL_VAR
        TokenType Type = TokenType::NO_TOKEN_VALUE; // optional, for future type-checking

        size_t OffsetInBytecode = 0; // Offset based off the compilation unit the compilationunit base offset will be recorded by the linker for resolving symbols

        uint8_t Flags = SymbolFlag::NO_FLAGS;

        Symbol(const string& fp_Name, SymbolKind fp_Kind, TokenType fp_Type, const size_t fp_Offset, const uint8_t fp_SymFlags)
            : Name(fp_Name), Kind(fp_Kind), Type(fp_Type), OffsetInBytecode(fp_Offset), Flags(fp_SymFlags) {
        }

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

    struct BongoCompiler
    {
        ~BongoCompiler() = default;

        BongoCompiler();

    public:
        shared_ptr<Logger> compiler_logger = nullptr; //shared for now cause idk how else work

    ///////////////////////////////////////////////////////////// PRIVATE /////////////////////////////////////////////////////////////
    private:
        unique_ptr<Parser> pm_BongoParser = nullptr; //needa make this a class since that's the only way cpp will let me do mutual recursion for some reason lmao

        uint64_t pm_CompilerID = 0;
        
        string pm_CompilerName = "NO_COMPILER_NAME";
        uint8_t pm_NextAvailableStackSlot = 0;

    private:
        ////////////////////////////////////////////// Utility Functions //////////////////////////////////////////////
        
        unique_ptr<StatementNode>
            ShiftForward(vector<unique_ptr<StatementNode>>& fp_ProgramBody);

        bool
            ReadFileIntoString
            (
                string* fp_SourceCode,
                const string& fp_ScriptFilePath
            )
            const;
        
        ////////////////////////////////////////////// Encoding Functions //////////////////////////////////////////////
        
        void
            Encode32BitInt(vector<uint8_t>& fp_ByteCode, uint32_t fp_Int);

        void
            Encode64BitInt(vector<uint8_t>& fp_ByteCode, uint64_t fp_Int);

        void
            EncodeUTF8String(vector<uint8_t>& fp_ByteCode, const string& fp_String);

        void
            EncodeFloat(vector<uint8_t>& fp_ByteCode, float fp_Float);

        void
            EncodeDouble
            (
                vector<uint8_t>& fp_ByteCode,
                const double fp_DoubleVal
            );

        void
            Encode32BitChar(vector<uint8_t>& fp_ByteCode, uint32_t character);

        void
            EncodeBool(vector<uint8_t>& fp_ByteCode, bool fp_Bool);

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
                PrintFunction* fp_PrintFunction,
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

        void
            UpdateThreadOwner()
            const
        {
            compiler_logger->UpdateThreadOwner();
        }
    };
}