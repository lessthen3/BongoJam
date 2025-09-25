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
#include "../../../include/bongo_lang/compiler/Compiler.h"

namespace BongoJam{

    BongoCompiler::BongoCompiler()
    {
        //pm_CompilerID = 2;

        compiler_logger = make_shared<Logger>();
        compiler_logger->Initialize(DEFAULT_LOG_OUTPUT_DIRECTORY, "BongoCompilerLogger", DEFAULT_LOG_LEVEL_FILTER);

        compiler_logger->Debug(format("Successfully initialized for Compiler with ID : {}", pm_CompilerID), "Compiler");

        pm_BongoParser = make_unique<Parser>(compiler_logger);
    }

//////////////////////////////////////////////
// Utility Functions
//////////////////////////////////////////////

unique_ptr<StatementNode>
    BongoCompiler::ShiftForward(vector<unique_ptr<StatementNode>>& fp_ProgramBody)
{
    if (fp_ProgramBody.empty())
    {
        return make_unique<StatementNode>(); //return escape char when source code is done being read
    }

    unique_ptr<StatementNode> f_FirstElement = move(fp_ProgramBody.front());
    fp_ProgramBody.erase(fp_ProgramBody.begin());

    return move(f_FirstElement);
}

bool
    BongoCompiler::ReadFileIntoString
    (
        string* fp_SourceCode,
        const string& fp_ScriptFilePath
    )
    const
{
    //check for nullptr
    if (not fp_SourceCode)
    {
        compiler_logger->Error("Nullptr reference passed to ReadFileIntoString()", "Compiler");
        return false;
    }
    // Ensure directory exists
    else if (not filesystem::exists(fp_ScriptFilePath))
    {
        compiler_logger->Error("Tried to pass invalid filepath to ReadFileIntoString()", "Compiler");
        return false;
    }

    // Extract file extension assuming format "filename.ext"
    size_t lastDotIndex = fp_ScriptFilePath.rfind('.');

    if (lastDotIndex == string::npos)
    {
        compiler_logger->Error("Compiler Error: No file extension found", "Compiler");
        return false;
    }

    string f_FileExtension = fp_ScriptFilePath.substr(lastDotIndex);

    if (f_FileExtension != ".bj")
    {
        compiler_logger->Error("Compiler Error: Please only try to compile .bj files", "Compiler");
        return false;
    }

    ifstream f_FileStream(fp_ScriptFilePath);

    if (not f_FileStream)
    {
        compiler_logger->Error("Compiler Error: Failed to open bongojam script for reading.", "Compiler");
        return false;
    }

    stringstream f_Buffer;
    f_Buffer << f_FileStream.rdbuf();
    *fp_SourceCode = f_Buffer.str();

    return true;
}

//////////////////////////////////////////////
// Encoding Functions
//////////////////////////////////////////////

void
    BongoCompiler::Encode32BitInt(vector<uint8_t>& fp_ByteCode, uint32_t fp_Int)
{
    fp_ByteCode.push_back((fp_Int >> 24) & 0xFF); // High byte
    fp_ByteCode.push_back((fp_Int >> 16) & 0xFF);
    fp_ByteCode.push_back((fp_Int >> 8) & 0xFF);
    fp_ByteCode.push_back(fp_Int & 0xFF);         // Low byte
}

void
    BongoCompiler::Encode64BitInt(vector<uint8_t>& fp_ByteCode, uint64_t fp_Int)
{
    fp_ByteCode.push_back((fp_Int >> 56) & 0xFF); // High byte
    fp_ByteCode.push_back((fp_Int >> 48) & 0xFF);
    fp_ByteCode.push_back((fp_Int >> 40) & 0xFF);
    fp_ByteCode.push_back((fp_Int >> 32) & 0xFF);         
    fp_ByteCode.push_back((fp_Int >> 24) & 0xFF);
    fp_ByteCode.push_back((fp_Int >> 16) & 0xFF);
    fp_ByteCode.push_back((fp_Int >> 8) & 0xFF);
    fp_ByteCode.push_back(fp_Int & 0xFF);         // Low byte
}

void
    BongoCompiler::EncodeUTF8String(vector<uint8_t>& fp_ByteCode, const string& fp_String)
{
    vector<uint8_t> f_EncodedBytes; // Temporary buffer to hold encoded bytes
    uint32_t f_SizeOfString = fp_String.size();

    //////////// next 32 bits are size of the string arg WIP
    // f_BongoProgram.ParsedScript
    // Encode each character in the string
    for (char _c : fp_String)
    {
        switch (_c)
        {
        case '\n':  // Newline
            f_EncodedBytes.push_back('\\');
            f_EncodedBytes.push_back('n');
            break;
        case '\t':  // Tab
            f_EncodedBytes.push_back('\\');
            f_EncodedBytes.push_back('t');
            break;
        case '\\':  // Backslash
            f_EncodedBytes.push_back('\\');
            f_EncodedBytes.push_back('\\');
            cout << "what the" << "\n";
            break;
        default:
            f_EncodedBytes.push_back(static_cast<uint8_t>(_c));
            break;
        }
    }

    Encode32BitInt(fp_ByteCode, static_cast<uint32_t>(f_EncodedBytes.size())); // Store the length of the string
    fp_ByteCode.insert(fp_ByteCode.end(), f_EncodedBytes.begin(), f_EncodedBytes.end());
}

void
    BongoCompiler::EncodeFloat(vector<uint8_t>& fp_ByteCode, float fp_Float)
{
    uint32_t asInt;
    memcpy(&asInt, &fp_Float, sizeof(float)); // Copy the float into an uint32_t bit pattern
    Encode32BitInt(fp_ByteCode, asInt);      // Reuse the integer encoding function
}

void
    BongoCompiler::Encode32BitChar(vector<uint8_t>& fp_ByteCode, uint32_t character)
{
    Encode32BitInt(fp_ByteCode, character); // Treat the character as a 32-bit integer
}

void
    BongoCompiler::EncodeBool(vector<uint8_t>& fp_ByteCode, bool fp_Bool)
{
    uint32_t boolAsInt = fp_Bool ? 1 : 0; // Convert boolean to 32-bit integer
    Encode32BitInt(fp_ByteCode, boolAsInt);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
    BongoCompiler::CompilePrintFunction
    (
        PrintFunction* fp_PrintFunction,
        CompilationUnit* fp_CompilationUnit
    )
{
    //////////////////////////////////////////////////////////// Bytecode ////////////////////////////////////////////////////////////

    fp_CompilationUnit->CompiledByteCode.push_back(STDOUT); //print function opcode
    string f_TextColour = "white";

    SingleValueExpr* f_StringVal = dynamic_cast<SingleValueExpr*>((fp_PrintFunction->m_FuncArgs[1]).get());
    f_TextColour = (f_StringVal->m_Value.m_Value);//only handling string literals for now

    //////////////////// Parse Arguments ////////////////////

    fp_CompilationUnit->CompiledByteCode.push_back(STRING_VALUE); //print function opcode
    EncodeUTF8String
    (
        fp_CompilationUnit->CompiledByteCode,
        CreateColouredText
        (
            (dynamic_cast<SingleValueExpr*>((fp_PrintFunction->m_FuncArgs[0]).get()))->m_Value.m_Value,
            f_TextColour
        )
    );

    //////////////////////////////////////////////////////////// SSA ////////////////////////////////////////////////////////////

    return true;
}

bool
    BongoCompiler::CompileDeclaredFunction
(
    FuncDeclaration* fp_FuncDeclaration,
    CompilationUnit* fp_CompilationUnit
)
{
    unique_ptr<StatementNode> f_CurrentProgramStatement = nullptr;

    while (fp_FuncDeclaration->CodeBody.size() > 0) //compiling the main function code body
    {
        f_CurrentProgramStatement = ShiftForward(fp_FuncDeclaration->CodeBody); //cool af it gets used here too uwu

        switch (f_CurrentProgramStatement->m_Domain)
        {
        case SyntaxNodeType::PrintFunction:
        {
            PrintFunction* sv_PrintFunction = dynamic_cast<PrintFunction*>(f_CurrentProgramStatement.get());
            if (not CompilePrintFunction(sv_PrintFunction, fp_CompilationUnit))
            {

                return false;
            }
            continue;
        }
        break;
        case SyntaxNodeType::VarDeclaration:
        {
            VarDeclaration* f_VarDeclaration = dynamic_cast<VarDeclaration*>(f_CurrentProgramStatement.get());
            if (not CompileVarDeclaration(f_VarDeclaration, fp_CompilationUnit))
            {

                return false;
            }
            continue;
        }
        break;
        default:
            compiler_logger->Error(format("Invalid statement unknown to compiler found inside the declaration of function: '{}' ", fp_FuncDeclaration->m_FuncName.m_Value), "BongoCompiler");
            break;
        }
    }

    return true;
}

bool
    BongoCompiler::CompileStructDeclaration
(
    StructDeclaration* fp_StructDec,
    CompilationUnit* fp_CompilationUnit
)
{

    for (auto& lv_Constructor : fp_StructDec->Constructors) //structs are only allowed to have constructors thats it uwu - maybeeeee idk
    {
        if (not CompileDeclaredFunction(&lv_Constructor, fp_CompilationUnit))
        {

            return false;
        }
    }

    return true;
}

bool
    BongoCompiler::CompileVarDeclaration
    (
        VarDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit
    )
{

    return true;
}

bool
    BongoCompiler::CompileFieldDeclaration
    (
        FieldDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit
    )
{

    return true;
}

bool
    BongoCompiler::CompileDeclaredClass
    (
        ClassDeclaration* fp_ClassDec,
        CompilationUnit* fp_CompilationUnit,
        const string& fp_NameSpace
    )
{
    for (auto& lv_Constructor : fp_ClassDec->Constructors) //compile constructors
    {
        //put symbol and offset before so the offset represents the first byte of the translated constructor declaration
        fp_CompilationUnit->SymbolTable.emplace
        (
            fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value + "::" + lv_Constructor.m_FuncName.m_Value, //resolves as classname::method
            Symbol
            (
                lv_Constructor.m_FuncName.m_Value,
                SymbolKind::Method,
                fp_ClassDec->SymbolTable.at(lv_Constructor.m_FuncName.m_Value).m_Type,
                fp_CompilationUnit->CompiledByteCode.size()
            )
        ); //idk if the bytecode part is kosher

        if (not CompileDeclaredFunction(&lv_Constructor, fp_CompilationUnit))
        {

            return false;
        }
    }

    for (auto& lv_Method : fp_ClassDec->Methods) //compile methods
    {
        fp_CompilationUnit->SymbolTable.emplace
        (
            fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value + "::" + lv_Method.m_FuncName.m_Value, //resolves as classname::method
            Symbol
            (
                lv_Method.m_FuncName.m_Value,
                SymbolKind::Method,
                fp_ClassDec->SymbolTable.at(lv_Method.m_FuncName.m_Value).m_Type,
                fp_CompilationUnit->CompiledByteCode.size()
            )
        ); //idk if the bytecode part is kosher

        if (not CompileDeclaredFunction(&lv_Method, fp_CompilationUnit))
        {

            return false;
        }
    }

    for (auto& lv_Field : fp_ClassDec->Fields)
    {
        fp_CompilationUnit->SymbolTable.emplace
        (
            fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value + "::" + lv_Field->Name.m_Value, //resolves as classname::method
            Symbol
            (
                lv_Field->Name.m_Value,
                SymbolKind::Field,
                fp_ClassDec->SymbolTable.at(lv_Field->Name.m_Value).m_Type,
                fp_CompilationUnit->CompiledByteCode.size()
            )
        ); //idk if the bytecode part is kosher        
        if (not CompileFieldDeclaration(lv_Field.get(), fp_CompilationUnit))
        {

            return false;
        }
    }

    return true;
}

void
    BongoCompiler::UpdateThreadOwner()
{
    compiler_logger->UpdateThreadOwner();
}

//////////////////////////////////////////////
// MAIN COMPILING FUNCTION
//////////////////////////////////////////////

int
    BongoCompiler::CompileUnit
    (
        const string& fp_BongoScriptFilePath,
        CompilationUnit* fp_CompilationUnit,
        const bool fp_IsDebug
    )
{
    //////////////////// Read .bj file, Tokenize and Parse it ////////////////////
    string f_SourceCode;
    if (not ReadFileIntoString(&f_SourceCode, fp_BongoScriptFilePath))
    {
        //stop compilation immediately
        compiler_logger->Fatal("Compiler was not able to read a valid source file, compilation will not proceed any further. nothing was done.", "Compiler");
        return BONGO_COMPILER_FAILED_TO_READ_SOURCE;
    }

    vector<Token> f_ProgramTokens;
    Tokenize(f_SourceCode, f_ProgramTokens, compiler_logger.get());

    unique_ptr<Program> f_BongoProgram = pm_BongoParser->ConstructAST(f_ProgramTokens); //doesnt need to be heap alloc'd since its just a bunch of vectors and maps

    if (not f_BongoProgram)
    {
        compiler_logger->Fatal(format("Failed to parse file: '{}', compilation failed :'(", fp_BongoScriptFilePath), "BongoCompiler");
        return EXIT_FAILURE;
    }

    // 
    //////////////////// READ AND HANDLE INCLUDES HERE ////////////////////

    //////////////////// Define Main Loop Variables ////////////////////

    size_t f_ProgramCounter = 0; //idk why but ill keep track of where we are -- future ryan: was hella useful for bytecode offset uwu but useless in parser since its a tree OwO

    bool f_ShouldShift = true;

    unique_ptr<StatementNode> f_CurrentProgramStatement;

    //////////////////// Main Compile Loop ////////////////////

    while (f_BongoProgram->ParsedScript.size() > 0) //compiling the main function code body
    {
        f_CurrentProgramStatement = ShiftForward(f_BongoProgram->ParsedScript);
        f_ProgramCounter++;

        switch (f_CurrentProgramStatement->m_Domain) //unique ptr's get thrown out each scope and cleaned up for me uwu
        {
        case SyntaxNodeType::FuncDeclaration:
        {
            unique_ptr<FuncDeclaration> f_FuncDec = unique_dynamic_cast<FuncDeclaration>(move(f_CurrentProgramStatement));
            
            if (not CompileDeclaredFunction(f_FuncDec.get(), fp_CompilationUnit))
            {

                return EXIT_FAILURE;
            }
        }
        break;
        case SyntaxNodeType::ClassDeclaration:
        {
            unique_ptr<ClassDeclaration> f_ClassDec = unique_dynamic_cast<ClassDeclaration>(move(f_CurrentProgramStatement));

            if(not CompileDeclaredClass(f_ClassDec.get(), fp_CompilationUnit))
            {

                return EXIT_FAILURE;
            }
        }
        break;
        case SyntaxNodeType::StructDeclaration:
        {
            unique_ptr<StructDeclaration> f_StructDec = unique_dynamic_cast<StructDeclaration>(move(f_CurrentProgramStatement));

            if (not CompileStructDeclaration(f_StructDec.get(), fp_CompilationUnit))
            {

                return EXIT_FAILURE;
            }
        }
        break;
        case SyntaxNodeType::NameSpace:
        {
            unique_ptr<StructDeclaration> f_StructDec = unique_dynamic_cast<StructDeclaration>(move(f_CurrentProgramStatement));

            if (not CompileStructDeclaration(f_StructDec.get(), fp_CompilationUnit))
            {

                return EXIT_FAILURE;
            }
        }
        break;
        case SyntaxNodeType::VarDeclaration:
        {
            unique_ptr<StructDeclaration> f_StructDec = unique_dynamic_cast<StructDeclaration>(move(f_CurrentProgramStatement));

            if (not CompileStructDeclaration(f_StructDec.get(), fp_CompilationUnit))
            {

                return EXIT_FAILURE;
            }
        }
        break;
        default:
            compiler_logger->Fatal(format("FATAL COMPILATION ERROR: Compiler tried processing an invalid StatementNode either produced improperly by Parser, or Compiler should know the statement but hasnt been updated properly\n COMPILER ID: {}\n", pm_CompilerID), "Compiler");
            break;
        }

    }

    return BONGO_OK;
}
}//namespace BongoJam