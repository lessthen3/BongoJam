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

///BongoJam
#include "Parser.h"
#include "../BongoGlue.h"

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

}

namespace BongoJam {

    enum class SymbolKind { Variable, Field, Function, Method, Struct, Class };

    struct Symbol
    {
        string Name;

        SymbolKind Kind; // FUNCTION, STRUCT, CLASS, GLOBAL_VAR
        TokenType Type; // optional, for future type-checking

        size_t OffsetInBytecode; // or StructLayout offset

        bool IsResolved = false;

        Symbol(const string& fp_Name, SymbolKind fp_Kind, TokenType fp_Type, size_t fp_Offset, bool fp_IsResolved = false) 
            : Name(fp_Name), Kind(fp_Kind), Type(fp_Type), OffsetInBytecode(fp_Offset), IsResolved(fp_IsResolved) {}
    };

    struct CompilationUnit
    {
        string ScriptPath; //compilation units rae generated per script so # of scripts = # of compilation units

        vector<uint8_t> CompiledByteCode; //bytecode
        vector<SSAInstruction> SSA_IR; //

        unordered_map<string, Symbol> SymbolTable; // symbol name : symbol information
        unordered_map<string, Symbol> UnresolvedSymbolTable; //hf linker

        //debugname table quesiton mark???_????
    };

    struct BongoScriptUnit
    {
        filesystem::path FilePath;
        unique_ptr<CompilationUnit> CompiledUnit = make_unique<CompilationUnit>();
    };

    struct BongoCompiler
    {
        ~BongoCompiler() = default;

        BongoCompiler();

    public:
        const string BONGO_VERSION = "0.0.1";

        shared_ptr<Logger> compiler_logger = nullptr; //shared for now cause idk how else work

    ///////////////////////////////////////////////////////////// PRIVATE /////////////////////////////////////////////////////////////
    private:
        unique_ptr<Parser> pm_BongoParser = nullptr; //needa make this a class since that's the only way cpp will let me do mutual recursion for some reason lmao

        uint64_t pm_CompilerID = 0;
        
        string pm_CompilerName;
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
            Encode32BitChar(vector<uint8_t>& fp_ByteCode, uint32_t character);

        void
            EncodeBool(vector<uint8_t>& fp_ByteCode, bool fp_Bool);

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
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileDeclaredClass
            (
                ClassDeclaration* fp_FuncDeclaration,
                CompilationUnit* fp_CompilationUnit,
                const string& fp_NameSpace = ""
            );

        bool
            CompileStructDeclaration
            (
                StructDeclaration* fp_StructDec,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileVarDeclaration
            (
                VarDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        bool
            CompileFieldDeclaration
            (
                FieldDeclaration* fp_VarDeclaration,
                CompilationUnit* fp_CompilationUnit
            );

        void
            UpdateThreadOwner();

        template<typename T>
        unique_ptr<T> unique_dynamic_cast(unique_ptr<StatementNode>&& base) 
        {
            T* derived = dynamic_cast<T*>(base.release());
            return unique_ptr<T>(derived);
        }

        //auto f_FuncDec = unique_dynamic_cast<FuncDeclaration>(move(f_CurrentProgramStatement));

    };
}