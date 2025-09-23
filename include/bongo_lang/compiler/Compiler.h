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

}

namespace BongoJam {

    //////////////////////////////////////////////
    // Import Verification (warn circular dependencies)
    //////////////////////////////////////////////

    struct ImportTree
    {

        bool
            ValidateImportTree()
        {
            return false;
        }
    };

    struct BONGO_WORD
    {
        OPCODES OP = OPCODES::NOP;
        unique_ptr<StatementNode> OPERAND = nullptr; //ptr to an AST statement, statements ar designed to be one complete "sentence" in a sense
    };

    struct CompilationUnit
    {
        vector<uint8_t> CompiledByteCode;

        vector<BONGO_WORD> CompiledCode;
        map<uint64_t, string> NameTable; // symbol ID : name characters
    };

    struct BongoCompiler
    {
        ~BongoCompiler() = default;

        BongoCompiler();

    public:
        const string BONGO_VERSION = "0.0.1";

    ///////////////////////////////////////////////////////////// PRIVATE /////////////////////////////////////////////////////////////
    private:
        unique_ptr<Parser> pm_BongoParser = nullptr; //needa make this a class since that's the only way cpp will let me do mutual recursion for some reason lmao

        unique_ptr<Logger> compiler_logger = nullptr;

        uint64_t pm_CompilerID = 0;

    private:
        ////////////////////////////////////////////// Utility Functions //////////////////////////////////////////////
        
        unique_ptr<StatementNode>
            ShiftForward(vector<unique_ptr<StatementNode>>& fp_ProgramBody);

        bool
            ReadFileIntoString
            (
                string* fp_SourceCode,
                const string& fp_ScriptFilePath
            );
        
        ////////////////////////////////////////////// Encoding Functions //////////////////////////////////////////////
        
        void
            Encode32BitInt(vector<uint8_t>& fp_ByteCode, uint32_t fp_Int);

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

        void
            CompilePrintFunction
            (
                PrintFunction* fp_PrintFunction,
                CompilationUnit* fp_CompilationUnit
            );

        void
            CompileDeclaredFunction
            (

            );

        template<typename T>
        std::unique_ptr<T> unique_dynamic_cast(std::unique_ptr<StatementNode>&& base) {
            T* derived = dynamic_cast<T*>(base.release());
            return std::unique_ptr<T>(derived);
        }
        //auto f_FuncDec = unique_dynamic_cast<FuncDeclaration>(move(f_CurrentProgramStatement));

    };
}