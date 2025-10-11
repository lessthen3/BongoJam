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
    BongoCompiler::Encode32BitInt
    (
        vector<uint8_t>& fp_ByteCode, 
        const uint32_t fp_Int
    )
{
    fp_ByteCode.push_back((fp_Int >> 24) & 0xFF); // High byte
    fp_ByteCode.push_back((fp_Int >> 16) & 0xFF);
    fp_ByteCode.push_back((fp_Int >> 8) & 0xFF);
    fp_ByteCode.push_back(fp_Int & 0xFF);         // Low byte
}

void
    BongoCompiler::Encode64BitInt
    (
        vector<uint8_t>& fp_ByteCode, 
        const uint64_t fp_Int
    )
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
    BongoCompiler::EncodeUTF8String
    (
        vector<uint8_t>& fp_ByteCode, 
        const string& fp_String
    )
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
    BongoCompiler::EncodeFloat
    (
        vector<uint8_t>& fp_ByteCode, 
        float fp_Float
    )
{
    uint32_t asInt;
    memcpy(&asInt, &fp_Float, sizeof(float)); // Copy the float into an uint32_t bit pattern
    Encode32BitInt(fp_ByteCode, asInt);      // Reuse the integer encoding function
}

void
    BongoCompiler::EncodeDouble
    (
        vector<uint8_t>& fp_ByteCode, 
        const double fp_DoubleVal
    )
{
    uint64_t asInt;
    memcpy(&asInt, &fp_DoubleVal, sizeof(double)); // Copy the double into a uint64_t bit pattern
    Encode64BitInt(fp_ByteCode, asInt);      // Reuse the integer encoding function
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
BongoCompiler::CompileFuncCall
(
    FunctionCallExpr* fp_VarDeclaration,
    CompilationUnit* fp_CompilationUnit
)
{

    return true;
}

//this function parses expression trees recursively to produce bytecode consistent with operation order and programmer's expected output
bool
    BongoCompiler::CompileRegularExpr //this is gonna be recursive i bet -check 
    (
        Expr* fp_Expression,
        CompilationUnit* fp_CompilationUnit
    )
{
    //Print(format("Current Expr type: {}", static_cast<int>(fp_Expression->m_Domain)), Colours::Magenta);

    switch (fp_Expression->m_Domain)
    {
    case SyntaxNodeType::SingleValueExpr: //each operator 
    {
        SingleValueExpr* sv_SingleValueExpr = dynamic_cast<SingleValueExpr*>(fp_Expression);

        switch (sv_SingleValueExpr->m_Value.m_Type)
        {
            case TokenType::FloatNumber:
            {
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::FLOAT_VALUE);
                EncodeFloat(fp_CompilationUnit->CompiledByteCode, stof(sv_SingleValueExpr->m_Value.m_Value));
            }
            break;
            case TokenType::IntNumber:
            {
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::INT_VALUE);
                Encode32BitInt(fp_CompilationUnit->CompiledByteCode, stoi(sv_SingleValueExpr->m_Value.m_Value));
            }
            break;
            case TokenType::StringLiteral:
            {
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::STRING_VALUE);
                EncodeUTF8String(fp_CompilationUnit->CompiledByteCode, sv_SingleValueExpr->m_Value.m_Value);
            }
            break;
            case TokenType::UnsignedIntNumber:
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::UNSIGNED_INT_VALUE);
                break;
            default: //THROW ERROR:
                compiler_logger->Error(format("Error at Line: {}, Invalid value found while compiling a single value expression OwO", sv_SingleValueExpr->m_Value.m_SourceCodeLineNumber), "BongoCompiler");
                return false;
        }
    }
    break;
    case SyntaxNodeType::ParenExpr: //each operator 
    {

    }
    break;
    case SyntaxNodeType::FunctionCallExpr:
    {

    }
    break;
    case SyntaxNodeType::IdentifierExpr:
    {

    }
    break;
    case SyntaxNodeType::BinaryOperationExpr:
    {
        BinaryOperationExpr* sv_BinaryOp = dynamic_cast<BinaryOperationExpr*>(fp_Expression);

        switch (sv_BinaryOp->m_Operator.m_Type)
        {
        case TokenType::AdditionOperator:
        {

        }
        break;
        case TokenType::NegativeOperator:
        {

        }
        break;
        case TokenType::DivisionOperator:
        {

        }
        break;
        case TokenType::MultiplicationOperator:
        {

        }
        break;
        case TokenType::ModulusOperator:
        {

        }
        break;
        default: //THROW ERROR: 
            
            return false;
        }
    }
    break;
    case SyntaxNodeType::UnaryOperatorExpr:
    {
        UnaryOperatorExpr* sv_BinaryOp = dynamic_cast<UnaryOperatorExpr*>(fp_Expression);

        switch (sv_BinaryOp->m_Operator.m_Type)
        {
        case TokenType::NegativeOperator:
        {

        }
        break;
        case TokenType::MinusMinusOperator:
        {

        }
        break;
        case TokenType::PlusPlusOperator:
        {

        }
        break;
        default: //THROW ERROR:
             
            return false;
        }
    }
    break;
    default: //THROW ERROR:

        return false;
    }

    return true; //return true since compiler knows the expression uwu
}

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
    BongoCompiler::CompileIfStatement
    (
        IfDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit
    )
{
    //////////////////////////////////////////////////////////// Bytecode ////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////// SSA ////////////////////////////////////////////////////////////////

    return true;
}

bool
    BongoCompiler::CompileWhileLoop
    (
        WhileLoopDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit
    )
{
    //////////////////////////////////////////////////////////// Bytecode ////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////// SSA ////////////////////////////////////////////////////////////////

    return true;
}

bool
    BongoCompiler::CompileForLoop
    (
        ForLoopDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit
    )
{
    //////////////////////////////////////////////////////////// Bytecode ////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////// SSA ////////////////////////////////////////////////////////////////

    return true;
}

bool
    BongoCompiler::CompileDeclaredFunction
(
    FuncDeclaration* fp_FuncDeclaration,
    CompilationUnit* fp_CompilationUnit,
    const string& fp_NameSpace
)
{
    fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::ENTER); //Create new stack frame

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
        }
        break;
        case SyntaxNodeType::VarDeclaration:
        {
            VarDeclaration* f_VarDeclaration = dynamic_cast<VarDeclaration*>(f_CurrentProgramStatement.get());
            if (not CompileVarDeclaration(f_VarDeclaration, fp_CompilationUnit))
            {

                return false;
            }
        }
        break;
        case SyntaxNodeType::ReturnSubStatement:
        {
            
        }
        break;
        case SyntaxNodeType::IfDeclaration:
        {
            IfDeclaration* f_IfDec = dynamic_cast<IfDeclaration*>(f_CurrentProgramStatement.get());

            if (not CompileIfStatement(f_IfDec, fp_CompilationUnit))
            {

                return false;
            }
        }
        break;
        case SyntaxNodeType::WhileLoopDeclaration:
        {

        }
        break;
        case SyntaxNodeType::ForLoopDeclaration:
        {

        }
        break;
        default:
            compiler_logger->Error(format("Invalid statement unknown to compiler found inside the declaration of function: '{}' ", fp_FuncDeclaration->m_FuncName.m_Value), "BongoCompiler");
            return false;
        }
    }

    fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::LEAVE); //pop stack frame

    return true;
}

bool
    BongoCompiler::CompileUserIdentifier //this is gonna be recursive i bet -check 
    (
        Expr* fp_SymbolExpr,
        CompilationUnit* fp_CompilationUnit
    )
{

    return true;
}

bool
    BongoCompiler::CompileVarReassignment //this is gonna be recursive i bet -check 
    (
        VariableReassignmentExpr* fp_Expression,
        CompilationUnit* fp_CompilationUnit
    )
{
    size_t f_EntryOffset = GetCurrentByteOffset(fp_CompilationUnit);

    //fp_CompilationUnit->SymbolTable.insert({ fp_Expression->VariableName-> });

    switch (fp_Expression->Operator)
    {
    case TokenType::PlusEqualsOperator: 
    {

    }
    break;
    case TokenType::MinusEqualsOperator: 
    {

    }
    break;
    case TokenType::DivEqualsOperator: 
    {

    }
    break;
    case TokenType::MultEqualsOperator: 
    {

    }
    break;
    case TokenType::ModuloEqualsOperator:
    {

    }
    break;
    case TokenType::BitAndEquals:
    {

    }
    break;
    case TokenType::BitOrEquals: //user defined type 
    {

    }
    break;
    default: //THROW ERROR: 

        return false;
    }
    //WARNING THIS WONT WORK ATM 
    string f_VarName;

    switch (fp_Expression->VariableName->m_Domain)
    {
    case SyntaxNodeType::SingleValueExpr:
    {
        auto sv_RecastedSingleValExpr = dynamic_cast<SingleValueExpr*>(fp_Expression->VariableName.get()); 

        f_VarName = sv_RecastedSingleValExpr->m_Value.m_Value;
    }
    break;
    case SyntaxNodeType::ContainerIndexedAccessExpr:
    {
        auto sv_RecastedContainerAcess = dynamic_cast<ContainerIndexedAccessExpr*>(fp_Expression->VariableName.get()); 

        f_VarName = sv_RecastedContainerAcess->ContainerName.m_Value;
    }
    break;
    default:

        return false;
    }

    //if the var is part of the current compilation unit then grab its symbol, otherwise var is calling to external compilation unit so the linker will resolve the symbol
    if (fp_CompilationUnit->SymbolTable.find(f_VarName) == fp_CompilationUnit->SymbolTable.end())
    {
        Symbol f_UnresolvedSymbol;

        f_UnresolvedSymbol.Name = f_VarName;
        f_UnresolvedSymbol.Type = fp_Expression->EvaluatesTo; //linker will check for type mistmatching uwu
        f_UnresolvedSymbol.OffsetInBytecode = f_EntryOffset; //record where it happened inside compilation unit uwu

        fp_CompilationUnit->UnresolvedSymbolTable.insert({ f_VarName, f_UnresolvedSymbol });

        return true;
    }

    Symbol  f_Symbol = fp_CompilationUnit->SymbolTable.at(f_VarName);

    if (f_Symbol.Type != fp_Expression->EvaluatesTo) //if the types dont match for storing and doing ops w uwu
    {

        return false;
    }


    if (f_Symbol.Flags & SymbolFlag::IS_HEAP)
    {
        fp_CompilationUnit->CompiledByteCode.push_back(STORE_GLOBAL);
    }
    else
    {
        fp_CompilationUnit->CompiledByteCode.push_back(STORE_LOCAL);
    }

    return true;
}

bool
    BongoCompiler::CompileVarDeclaration
    (
        VarDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit,
        const string& fp_NameSpace
    )
{
    fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::PUSH); //push new valus

    if (not CompileRegularExpr(fp_VarDeclaration->DefaultValue.get(), fp_CompilationUnit))
    {
        compiler_logger->Error(format("Error at Line Number: {}, unable to compile default value of variable named: {}", fp_VarDeclaration->Name.m_SourceCodeLineNumber, fp_VarDeclaration->Name.m_Value), "Compiler");
        return false;
    }

    fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::STORE_LOCAL); //push new valus

    return true;
}

bool
    BongoCompiler::CompileFieldDeclaration
    (
        VarDeclaration* fp_VarDeclaration,
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
        //fp_CompilationUnit->SymbolTable.emplace
        //(
        //    fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value + "::" + lv_Constructor->m_FuncName.m_Value, //resolves as classname::method
        //    Symbol
        //    (
        //        lv_Constructor->m_FuncName.m_Value,
        //        SymbolKind::Method,
        //        fp_ClassDec->SymbolTable.at(lv_Constructor->m_FuncName.m_Value).m_Type,
        //        fp_CompilationUnit->CompiledByteCode.size()
        //    )
        //); //idk if the bytecode part is kosher

        if (not CompileDeclaredFunction(lv_Constructor.get(), fp_CompilationUnit))
        {

            return false;
        }
    }

    for (auto& lv_Method : fp_ClassDec->Methods) //compile methods
    {
        //fp_CompilationUnit->SymbolTable.emplace
        //(
        //    fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value + "::" + lv_Method->m_FuncName.m_Value, //resolves as classname::method
        //    Symbol
        //    (
        //        lv_Method->m_FuncName.m_Value,
        //        SymbolKind::Method,
        //        fp_ClassDec->SymbolTable.at(lv_Method->m_FuncName.m_Value).m_Type,
        //        fp_CompilationUnit->CompiledByteCode.size()
        //    )
        //); //idk if the bytecode part is kosher

        if (not CompileDeclaredFunction(lv_Method.get(), fp_CompilationUnit))
        {

            return false;
        }
    }

    for (auto& lv_Field : fp_ClassDec->Fields)
    {
        //fp_CompilationUnit->SymbolTable.emplace
        //(
        //    fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value + "::" + lv_Field->Name.m_Value, //resolves as namespace::classname::method
        //    Symbol
        //    (
        //        lv_Field->Name.m_Value,
        //        SymbolKind::Field,
        //        fp_ClassDec->SymbolTable.at(lv_Field->Name.m_Value).m_Type,
        //        fp_CompilationUnit->CompiledByteCode.size()
        //    )
        //); //idk if the bytecode part is kosher        
        if (not CompileFieldDeclaration(lv_Field.get(), fp_CompilationUnit))
        {

            return false;
        }
    }

    return true;
}

bool
    BongoCompiler::CompileDeclaredStruct
    (
        StructDeclaration* fp_StructDec,
        CompilationUnit* fp_CompilationUnit,
        const string& fp_NameSpace
    )
{
    //////////////////////////////////////////////////////////// Bytecode ////////////////////////////////////////////////////////////

    for (auto& lv_Constructor : fp_StructDec->Constructors) //structs are only allowed to have constructors thats it uwu - maybeeeee idk
    {
        if (not CompileDeclaredFunction(&lv_Constructor, fp_CompilationUnit))
        {

            return false;
        }
    }

    //////////////////////////////////////////////////////////// SSA ////////////////////////////////////////////////////////////////


    return true;
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
    //////////////////// Catch nullptr ref ////////////////////

    if (not fp_CompilationUnit)
    {
        compiler_logger->Fatal(format("Tried to pass nullptr reference for CompilationUnit during attempted compilation of script: '{}'", fp_BongoScriptFilePath), "BongoCompiler");
        return EXIT_FAILURE;
    }

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

    //////////////////// Define Main Loop Variables ////////////////////

    size_t f_ProgramCounter = 0; //idk why but ill keep track of where we are -- future ryan: was hella useful for bytecode offset uwu but useless in parser since its a tree OwO

    bool f_ShouldShift = true;

    string f_CurrentNamespace; //track current namespace uwu

    unique_ptr<StatementNode> f_CurrentProgramStatement;

    //////////////////////////////////////////////////////////// Main Compile Loop ////////////////////////////////////////////////////////////

    while (f_BongoProgram->ParsedScript.size() > 0) //compiling the main function code body
    {
        f_CurrentProgramStatement = ShiftForward(f_BongoProgram->ParsedScript);
        f_ProgramCounter++;

        switch (f_CurrentProgramStatement->m_Domain) //unique ptr's get thrown out each scope and cleaned up for me uwu
        {
        case SyntaxNodeType::FuncDeclaration:
        {
            unique_ptr<FuncDeclaration> sv_FuncDec = unique_dynamic_cast<FuncDeclaration>(move(f_CurrentProgramStatement));
            
            if (not CompileDeclaredFunction(sv_FuncDec.get(), fp_CompilationUnit, f_CurrentNamespace))
            {
                compiler_logger->Error(format("Invalid statement unknown to compiler found inside the declaration of function: '{}' ", sv_FuncDec->m_FuncName.m_Value), "BongoCompiler");
                return EXIT_FAILURE;
            }
        }
        break;
        case SyntaxNodeType::ClassDeclaration:
        {
            unique_ptr<ClassDeclaration> sv_ClassDec = unique_dynamic_cast<ClassDeclaration>(move(f_CurrentProgramStatement));

            if(not CompileDeclaredClass(sv_ClassDec.get(), fp_CompilationUnit, f_CurrentNamespace))
            {
                compiler_logger->Error(format("Invalid statement unknown to compiler found inside the declaration of class: '{}' ", sv_ClassDec->ClassName.m_Value), "BongoCompiler");
                return EXIT_FAILURE;
            }

            Symbol f_Symbol; //symbol for a globally defined var within the namespace

            f_Symbol.Name = f_CurrentNamespace + sv_ClassDec->ClassName.m_Value;
            f_Symbol.OffsetInBytecode = GetCurrentByteOffset(fp_CompilationUnit);
            f_Symbol.Type = TokenType::UserIdentifier;
            f_Symbol.Kind = SymbolKind::Class;
            //f_Symbol.Flags = SymbolFlag::

            fp_CompilationUnit->SymbolTable.insert({ sv_ClassDec->ClassName.m_Value, f_Symbol });
        }
        break;
        case SyntaxNodeType::StructDeclaration:
        {
            unique_ptr<StructDeclaration> sv_StructDec = unique_dynamic_cast<StructDeclaration>(move(f_CurrentProgramStatement));

            if (not CompileDeclaredStruct(sv_StructDec.get(), fp_CompilationUnit, f_CurrentNamespace))
            {
                compiler_logger->Error(format("Invalid statement unknown to compiler found inside the declaration of class: '{}' ", sv_StructDec->StructName.m_Value), "BongoCompiler");
                return EXIT_FAILURE;
            }
        }
        break;
        case SyntaxNodeType::NameSpace:
        {
            unique_ptr<NamespaceDeclaration> sv_Namespace = unique_dynamic_cast<NamespaceDeclaration>(move(f_CurrentProgramStatement));

            f_CurrentNamespace = sv_Namespace->m_Name.m_Value;
        }
        break;
        case SyntaxNodeType::VarDeclaration: //global var
        {
            unique_ptr<VarDeclaration> sv_VarDec = unique_dynamic_cast<VarDeclaration>(move(f_CurrentProgramStatement));

            if (not CompileVarDeclaration(sv_VarDec.get(), fp_CompilationUnit, f_CurrentNamespace))
            {
                compiler_logger->Error(format("Invalid statement unknown to compiler found during the declaration of variable: '{}' ", sv_VarDec->Name.m_Value), "BongoCompiler");
                return EXIT_FAILURE;
            }

            Symbol f_Symbol; //symbol for a globally defined var within the namespace

            f_Symbol.Name = f_CurrentNamespace + sv_VarDec->Name.m_Value;
            f_Symbol.OffsetInBytecode = GetCurrentByteOffset(fp_CompilationUnit);
            f_Symbol.Type = sv_VarDec->Type.m_Type;
            f_Symbol.Kind = SymbolKind::Variable;
            //f_Symbol.Flags = SymbolFlag::

            fp_CompilationUnit->SymbolTable.insert({ sv_VarDec->Name.m_Value, f_Symbol });
        }
        break;
        case SyntaxNodeType::IncludeStatement:
        {
            unique_ptr<IncludeStatement> f_VarDec = unique_dynamic_cast<IncludeStatement>(move(f_CurrentProgramStatement));

            fp_CompilationUnit->Includes.push_back(f_VarDec->m_IncludePath.m_Value);
        }
        break;
        default:
            compiler_logger->Fatal(format("FATAL COMPILATION ERROR: Compiler tried processing an invalid StatementNode either produced improperly by Parser, or Compiler should know the statement but hasnt been updated properly\n COMPILER ID: {}\n", pm_CompilerID), "Compiler");
            return EXIT_FAILURE;
        }
    }

    return BONGO_OK;
}
}//namespace BongoJam