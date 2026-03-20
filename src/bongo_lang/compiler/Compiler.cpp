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
#include "Compiler.h"

namespace BongoJam{

    BongoCompiler::BongoCompiler(const string& fp_CompilerName)
    {
        stringstream f_UckCPlusPlus; //XXX: cpp is a dumb fucking language sometimes holy please make good features and not dumbass nonsense holy shit
        f_UckCPlusPlus << this_thread::get_id();
        string f_CallerThreadID = f_UckCPlusPlus.str();

        pm_CompiledThreadID = stoull(f_CallerThreadID);

        pm_CompilerName = fp_CompilerName;

        compiler_logger = Logger::CreateShared(pm_CompilerName, DEFAULT_LOG_FLAGS, DEFAULT_LOG_OUTPUT_DIRECTORY);

        if (not compiler_logger)
        {
            throw runtime_error("Failed to initialize logger for " + pm_CompilerName);
        }

        compiler_logger->Debug(format("Successfully initialized for Compiler with ID : {}", pm_CompiledThreadID), "Compiler");

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

    unique_ptr<StatementNode> f_FirstElement = std::move(fp_ProgramBody.front());
    fp_ProgramBody.erase(fp_ProgramBody.begin());

    return f_FirstElement;
}

bool
    BongoCompiler::TryToResolveSymbol(const string& fp_SymbolName, CompilationUnit* fp_CompilationUnit)
{
    return fp_CompilationUnit->SymbolTable.find(fp_SymbolName) == fp_CompilationUnit->SymbolTable.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
    BongoCompiler::CompileFuncCall
    (
        FunctionCallExpr* fp_FunctionCallExpr,
        CompilationUnit* fp_CompilationUnit
    )
{
    auto f_Iterator = STANDARD_FUNCTIONS.find(fp_FunctionCallExpr->FuncName.m_Value);

    TokenType f_CallType;

    if (f_Iterator == STANDARD_FUNCTIONS.end())
    {
        f_CallType = TokenType::UserIdentifier;
    }
    else
    {
        f_CallType = f_Iterator->second;
    }

    switch (f_CallType)
    {
    case TokenType::Input:
    {
        if (not CompileInputFunction(fp_FunctionCallExpr, fp_CompilationUnit))
        {
            compiler_logger->Error(format("Error at Line: {}, Unable to compile input() function oof", fp_FunctionCallExpr->FuncName.m_SourceCodeLineNumber), "BongoCompiler");
            return false;
        }
    }
    break;
    case TokenType::Print:
    {
        if (not CompilePrintFunction(fp_FunctionCallExpr, fp_CompilationUnit))
        {
            compiler_logger->Error(format("Error at Line: {}, Unable to compile print() function oof", fp_FunctionCallExpr->FuncName.m_SourceCodeLineNumber), "BongoCompiler");
            return false;
        }
    }
    break;
    case TokenType::UserIdentifier:
    {
        //check for the symbol to make sure it exists
        if (TryToResolveSymbol(fp_FunctionCallExpr->FuncName.m_Value, fp_CompilationUnit))
        {

        }
    }
    break;
    default:

        return false;
    }

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
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::PUSH_F);
                BinaryCodec::EncodeDouble(fp_CompilationUnit->CompiledByteCode, stod(sv_SingleValueExpr->m_Value.m_Value));
            }
            break;
            case TokenType::IntNumber:
            {
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::PUSH_I);
                BinaryCodec::EncodeInt<int64_t>(fp_CompilationUnit->CompiledByteCode, stoll(sv_SingleValueExpr->m_Value.m_Value));
            }
            break;
            case TokenType::StringLiteral:
            {
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::PUSH_S);
                BinaryCodec::EncodeStringUTF8<uint32_t>(fp_CompilationUnit->CompiledByteCode, sv_SingleValueExpr->m_Value.m_Value);
            }
            break;
            case TokenType::UnsignedIntNumber:
            {
                fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::PUSH_U);
                BinaryCodec::EncodeInt<uint64_t>(fp_CompilationUnit->CompiledByteCode, stoull(sv_SingleValueExpr->m_Value.m_Value));
            }
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
        FunctionCallExpr* sv_FunctionCallExpr = dynamic_cast<FunctionCallExpr*>(fp_Expression);

        if (not CompileFuncCall(sv_FunctionCallExpr, fp_CompilationUnit))
        {

            return false;
        }
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
        FunctionCallExpr* fp_PrintFunction,
        CompilationUnit* fp_CompilationUnit
    )
{
    if (fp_PrintFunction->Arguments.size() != 1)
    {
        compiler_logger->Error(format("Error at Line Number: {}, invalid argument count found when compiling print() function call", fp_PrintFunction->FuncName.m_SourceCodeLineNumber), "Compiler");
        return false;
    }

    //////////////////////////////////////////////////////////// Bytecode ////////////////////////////////////////////////////////////

    fp_CompilationUnit->CompiledByteCode.push_back(STDOUT); //print function opcode

    SingleValueExpr* f_StringVal = dynamic_cast<SingleValueExpr*>(fp_PrintFunction->Arguments[0].get());

    //WARNING: just assuming single val expr strings atm need to rework this w a switch to handle more complicated expressions using CompileRegularExpr()
    //fp_CompilationUnit->CompiledByteCode.push_back(STRING_VALUE); //print function opcode

    string f_PrintString;

    if (f_StringVal->Decorator.m_Type != TokenType::NO_TOKEN_VALUE)
    {
        f_PrintString = CreateColouredText(f_StringVal->m_Value.m_Value, f_StringVal->Decorator.m_Value);
    }
    else
    {
        f_PrintString = f_StringVal->m_Value.m_Value;
    }

    //////////////////// Encode String UwU ////////////////////

    BinaryCodec::EncodeStringUTF8<uint32_t>
    (
        fp_CompilationUnit->CompiledByteCode,
        f_PrintString
    );

    //////////////////////////////////////////////////////////// SSA ////////////////////////////////////////////////////////////

    return true;
}

[[nodiscard]] bool
    BongoCompiler::CompileStringExpr
    (
        Expr* fp_Expression,
        CompilationUnit* fp_CompilationUnit
    )
{
    string f_PrintString;

    switch (fp_Expression->m_Domain)
    {
    case SyntaxNodeType::SingleValueExpr:
    {
        //////////////////// Recast to single val expr ////////////////////

        SingleValueExpr* f_StringVal = dynamic_cast<SingleValueExpr*>(fp_Expression);

        //////////////////// Look for colour decorator ////////////////////

        if (f_StringVal->Decorator.m_Type != TokenType::NO_TOKEN_VALUE)
        {
            //////////////////// Encode String UwU ////////////////////

            BinaryCodec::EncodeStringUTF8<uint32_t>
            (
                fp_CompilationUnit->CompiledByteCode,
                CreateColouredText(f_StringVal->m_Value.m_Value, f_StringVal->Decorator.m_Value)
            );
        }
        else
        {
            BinaryCodec::EncodeStringUTF8<uint32_t>
            (
                fp_CompilationUnit->CompiledByteCode,
                f_StringVal->m_Value.m_Value
            );
        }
    }
    break;
    case SyntaxNodeType::IdentifierExpr:
    {
        IdentifierExpr* sv_IdentifierExpr = dynamic_cast<IdentifierExpr*>(fp_Expression);

        //TryToResolveSymbol()
    }
    break;
    case SyntaxNodeType::ContainerIndexedAccessExpr:
    {

    }
    break;
    case SyntaxNodeType::BinaryOperationExpr:
    {
        BinaryOperationExpr* f_AdditionExpr = dynamic_cast<BinaryOperationExpr*>(fp_Expression);

        if (f_AdditionExpr->m_Operator.m_Type != TokenType::AdditionOperator)
        {

            return false;
        }

        if (f_AdditionExpr->First->m_Domain == SyntaxNodeType::SingleValueExpr and f_AdditionExpr->Second->m_Domain == SyntaxNodeType::SingleValueExpr)
        {
            SingleValueExpr* f_FirstValue = dynamic_cast<SingleValueExpr*>(f_AdditionExpr->First.get());
            SingleValueExpr* f_SecondValue = dynamic_cast<SingleValueExpr*>(f_AdditionExpr->Second.get());

            if (f_FirstValue->Decorator.m_Type != TokenType::NO_TOKEN_VALUE)
            {
                f_PrintString += CreateColouredText(f_FirstValue->m_Value.m_Value, f_FirstValue->Decorator.m_Value);
            }
            else
            {
                f_PrintString += f_FirstValue->m_Value.m_Value;
            }
            if (f_SecondValue->Decorator.m_Type != TokenType::NO_TOKEN_VALUE)
            {
                f_PrintString += CreateColouredText(f_SecondValue->m_Value.m_Value, f_SecondValue->Decorator.m_Value);
            }
            else
            {
                f_PrintString += f_SecondValue->m_Value.m_Value;
            }
        }
        else if (f_AdditionExpr->First->m_Domain == SyntaxNodeType::IdentifierExpr)
        {

            if (not CompileStringExpr(f_AdditionExpr->Second.get(), fp_CompilationUnit))
            {

                return false;
            }
        }
        else if (f_AdditionExpr->First->m_Domain == SyntaxNodeType::ContainerIndexedAccessExpr)
        {

        }

    }
    break;
    default:
        compiler_logger->Error("Invalid string expression found!", "Compiler");
        break;
    }
    //WARNING: just assuming single val expr strings atm need to rework this w a switch to handle more complicated expressions using CompileRegularExpr()
    //fp_CompilationUnit->CompiledByteCode.push_back(STRING_VALUE); //print function opcode



    return true;
}

bool
    BongoCompiler::CompileInputFunction
    (
        FunctionCallExpr* fp_InputFunction,
        CompilationUnit* fp_CompilationUnit
    )
{
    if (fp_InputFunction->Arguments.size() == 1)
    {
        fp_CompilationUnit->CompiledByteCode.push_back(STDOUT); //print function opcode

        if (not CompileStringExpr(fp_InputFunction->Arguments[0].get(), fp_CompilationUnit))
        {
            compiler_logger->Error(format("Error at Line Number: {}, invalid string expression found when compiling input() function call", fp_InputFunction->FuncName.m_SourceCodeLineNumber), "Compiler");
            return false;
        }
    }
    else if (fp_InputFunction->Arguments.size() != 0)
    {
        compiler_logger->Error(format("Error at Line Number: {}, invalid number of arguments, found: {} arguments when 1 or 0 was expected when compiling input() function call", fp_InputFunction->FuncName.m_SourceCodeLineNumber, fp_InputFunction->Arguments.size()), "Compiler");
        return false;
    }

    fp_CompilationUnit->CompiledByteCode.push_back(STDIN); //print function opcode

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

    size_t f_InitialOffset = 0; //calculate stack slot offset and reuse across each branch


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
        case SyntaxNodeType::FunctionCallExpr:
        {
            FunctionCallExpr* sv_FunctionCallExpr = dynamic_cast<FunctionCallExpr*>(f_CurrentProgramStatement.get());
            CompileFuncCall(sv_FunctionCallExpr, fp_CompilationUnit);
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

    Symbol f_FuncSymbol;

    f_FuncSymbol.Name = fp_NameSpace  + "::" + fp_FuncDeclaration->m_FuncName.m_Value; //resolves as namespace::classname::method
    f_FuncSymbol.Kind = SymbolKind::Function;
    f_FuncSymbol.Type = TokenType::UserIdentifier;
    f_FuncSymbol.OffsetInBytecode = fp_CompilationUnit->CompiledByteCode.size();
    f_FuncSymbol.Flags = fp_FuncDeclaration->Modifiers;

    //put symbol and offset before so the offset represents the first byte of the translated constructor declaration
    fp_CompilationUnit->SymbolTable.insert({ f_FuncSymbol.Name, f_FuncSymbol });

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

    //WARNING THIS WONT WORK ATM 
    string f_VarName;

    switch (fp_Expression->VariableName->m_Domain)
    {
    case SyntaxNodeType::IdentifierExpr:
    {
        auto sv_IdentifierExpr = dynamic_cast<IdentifierExpr*>(fp_Expression->VariableName.get()); 

        f_VarName = sv_IdentifierExpr->Identifier.m_Value;

        if(sv_IdentifierExpr->ChainedExpr != nullptr) //check for null since chained expr can be null
        {
            while (1)
            {
                switch (sv_IdentifierExpr->ChainedExpr->m_Domain)
                {
                case SyntaxNodeType::SingleValueExpr:
                {

                }
                break;
                case SyntaxNodeType::ContainerIndexedAccessExpr:
                {

                }
                break;
                case SyntaxNodeType::IdentifierExpr:
                {

                }
                break;
                default:
                    return false;
                    break;
                }
            }
        }
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

    //////////////////////////////////////////////////////////// check for equals since we don't need to load the val if its just a reg reassignment uwu ////////////////////////////////////////////////////////////

    if(fp_Expression->Operator.m_Type == TokenType::Equals) //do nothing since store will be called anyways
    {
        if (not CompileRegularExpr(fp_Expression->NewValue.get(), fp_CompilationUnit))
        {
            compiler_logger->Error(format("Error at Line Number: {}, unable to compile default value of variable named: {}", fp_Expression->Operator.m_SourceCodeLineNumber, f_VarName), "Compiler");
            return false;
        }
        
        if (f_Symbol.Kind & (SymbolKind::Class | SymbolKind::Struct))
        {
            fp_CompilationUnit->CompiledByteCode.push_back(STORE_GLOBAL);
            //needa find address
        }
        else
        {
            fp_CompilationUnit->CompiledByteCode.push_back(STORE_LOCAL);
            BinaryCodec::EncodeInt32(fp_CompilationUnit->CompiledByteCode, f_Symbol.Slot); //find slot in stack
        }

        return true;
    }

    //////////////////////////////////////////////////////////// Load Current Variable ////////////////////////////////////////////////////////////

    if (f_Symbol.Kind & (SymbolKind::Class | SymbolKind::Struct))
    {
        fp_CompilationUnit->CompiledByteCode.push_back(LOAD_GLOBAL); //print function opcode
    }
    else
    {
        fp_CompilationUnit->CompiledByteCode.push_back(LOAD_LOCAL); //print function opcode
    }

    //////////////////////////////////////////////////////////// Figure out what the new value is supposed to be ////////////////////////////////////////////////////////////

    if (not CompileRegularExpr(fp_Expression->NewValue.get(), fp_CompilationUnit))
    {
        compiler_logger->Error(format("Error at Line Number: {}, unable to compile default value of variable named: {}", fp_Expression->Operator.m_SourceCodeLineNumber, f_VarName), "Compiler");
        return false;
    }

    //////////////////////////////////////////////////////////// Figure out which operator is being used ////////////////////////////////////////////////////////////

    switch (fp_Expression->Operator.m_Type)
    {
    case TokenType::PlusEqualsOperator:
    {
        //fp_CompilationUnit->CompiledByteCode.push_back(ADD); //print function opcode
    }
    break;
    case TokenType::MinusEqualsOperator:
    {
        //fp_CompilationUnit->CompiledByteCode.push_back(SUB); //print function opcode
    }
    break;
    case TokenType::DivEqualsOperator:
    {
        //fp_CompilationUnit->CompiledByteCode.push_back(DIV); 
    }
    break;
    case TokenType::MultEqualsOperator:
    {
        //fp_CompilationUnit->CompiledByteCode.push_back(MULT);
    }
    break;
    case TokenType::ModuloEqualsOperator:
    {
        //fp_CompilationUnit->CompiledByteCode.push_back(MOD);
    }
    break;
    case TokenType::BitAndEquals:
    {
        fp_CompilationUnit->CompiledByteCode.push_back(AND);
    }
    break;
    case TokenType::BitOrEquals: //user defined type 
    {
        fp_CompilationUnit->CompiledByteCode.push_back(OR);
    }
    break;
    case TokenType::BitXorEquals:
    {
        fp_CompilationUnit->CompiledByteCode.push_back(XOR);
    }
    break;
    default: //THROW ERROR: 
        compiler_logger->Error(format("Error at Line Number: {}, unable to compile default value of variable named: {}", fp_Expression->Operator.m_SourceCodeLineNumber, f_VarName), "Compiler");
        return false;
    }

    //////////////////////////////////////////////////////////// Store the result ////////////////////////////////////////////////////////////

    if (f_Symbol.Kind & (SymbolKind::Class | SymbolKind::Struct))
    {
        fp_CompilationUnit->CompiledByteCode.push_back(STORE_GLOBAL);
        //needa find address
    }
    else
    {
        fp_CompilationUnit->CompiledByteCode.push_back(STORE_LOCAL);
        BinaryCodec::EncodeInt32(fp_CompilationUnit->CompiledByteCode, f_Symbol.Slot); //find slot in stack
    }


    return true;
}

bool
    BongoCompiler::CompileVarDeclaration //used for global scope defined vars atm but wanna rework into class scoped vars probs just pass the class namespace thru
    (
        VarDeclaration* fp_VarDeclaration,
        CompilationUnit* fp_CompilationUnit,
        const string& fp_NameSpace
    )
{
    if (not CompileRegularExpr(fp_VarDeclaration->DefaultValue.get(), fp_CompilationUnit))
    {
        compiler_logger->Error(format("Error at Line Number: {}, unable to compile default value of variable named: {}", fp_VarDeclaration->Name.m_SourceCodeLineNumber, fp_VarDeclaration->Name.m_Value), "Compiler");
        return false;
    }

    fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::STORE_LOCAL); //push new valus

    BinaryCodec::EncodeInt32(fp_CompilationUnit->CompiledByteCode, pm_NextAvailableStackSlot);

    Symbol f_Symbol; 

    f_Symbol.Name = fp_NameSpace + "::" + fp_VarDeclaration->Name.m_Value;
    f_Symbol.OffsetInBytecode = GetCurrentByteOffset(fp_CompilationUnit);

    f_Symbol.Type = TokenType::UserIdentifier;
    f_Symbol.Kind = SymbolKind::Variable;
    f_Symbol.Flags = fp_VarDeclaration->Modifiers;

    f_Symbol.Slot = pm_NextAvailableStackSlot;

    pm_NextAvailableStackSlot++;

    fp_CompilationUnit->SymbolTable.insert({ fp_VarDeclaration->Name.m_Value, f_Symbol });

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
    const string f_ClassNamespace = fp_NameSpace + "::" + fp_ClassDec->ClassName.m_Value;

    for (auto& lv_Constructor : fp_ClassDec->Constructors) //compile constructors
    {
        if (not CompileDeclaredFunction(lv_Constructor.get(), fp_CompilationUnit, f_ClassNamespace))
        {

            return false;
        }
    }

    for (auto& lv_Method : fp_ClassDec->Methods) //compile methods
    {
        if (not CompileDeclaredFunction(lv_Method.get(), fp_CompilationUnit, f_ClassNamespace))
        {

            return false;
        }
    }

    for (auto& lv_Field : fp_ClassDec->Fields)
    {
        //if (not CompileFieldDeclaration(lv_Field.get(), fp_CompilationUnit, f_ClassNamespace))
        //{

        //    return false;
        //}
    }

    for (auto& lv_NestedClassDec : fp_ClassDec->NestedClassDecs)
    {
        if (not CompileDeclaredClass(lv_NestedClassDec.get(), fp_CompilationUnit, f_ClassNamespace))
        {

            return false;
        }
    }
    
    Symbol f_Symbol; //symbol for a globally defined var within the namespace

    f_Symbol.Name = f_ClassNamespace;
    f_Symbol.OffsetInBytecode = GetCurrentByteOffset(fp_CompilationUnit);
    f_Symbol.Type = TokenType::UserIdentifier;
    f_Symbol.Kind = SymbolKind::Class;
    //f_Symbol.Flags = SymbolFlag::

    fp_CompilationUnit->SymbolTable.insert({ fp_ClassDec->ClassName.m_Value, f_Symbol });

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
        return TRIED_TO_PASS_NULLPTR_REF_TO_COMPILATION_UNIT;
    }

    //////////////////// Read .bj file, Tokenize and Parse it ////////////////////

    vector<char> f_SourceCode;
    if (not FileIO::ReadFileIntoCharBuffer(fp_BongoScriptFilePath, {".bj", ".bjs"}, f_SourceCode, compiler_logger.get()))
    {
        //stop compilation immediately
        compiler_logger->Fatal("Compiler was not able to read a valid source file, compilation will not proceed any further. nothing was done.", "Compiler");
        return BONGO_COMPILER_FAILED_TO_READ_SOURCE;
    }

    vector<Token> f_ProgramTokens;
    if (not Tokenize(VectorStream<char>(move(f_SourceCode)), f_ProgramTokens, compiler_logger.get()))
    {
        compiler_logger->Fatal(format("Compiler was not able to Lex: '{}', compilation will not proceed any further. nothing was done.", fp_BongoScriptFilePath), "Compiler");
        return BONGO_FAILED_TO_LEX_SCRIPT;
    }

    unique_ptr<Program> f_BongoProgram = pm_BongoParser->ConstructAST(VectorStream<Token>(move(f_ProgramTokens))); //doesnt need to be heap alloc'd since its just a bunch of vectors and maps

    if (not f_BongoProgram)
    {
        compiler_logger->Fatal(format("Failed to parse file: '{}', compilation failed :'(", fp_BongoScriptFilePath), "BongoCompiler");
        return EXIT_FAILURE;
    }

    //////////////////// Define Main Loop Variables ////////////////////

    pm_NextAvailableStackSlot = 0; //reset for repeated compilation using the same compiler owo

    string f_CurrentNamespace; //track current namespace uwu

    unique_ptr<StatementNode> f_CurrentProgramStatement;

    //////////////////////////////////////////////////////////// Main Compile Loop ////////////////////////////////////////////////////////////

    while (f_BongoProgram->ParsedScript.size() > 0) //compiling the main function code body
    {
        f_CurrentProgramStatement = ShiftForward(f_BongoProgram->ParsedScript);

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
        }
        break;
        case SyntaxNodeType::IncludeStatement:
        {
            unique_ptr<IncludeStatement> f_VarDec = unique_dynamic_cast<IncludeStatement>(move(f_CurrentProgramStatement));

            fp_CompilationUnit->Includes.push_back(f_VarDec->m_IncludePath.m_Value);
        }
        break;
        default:
            compiler_logger->Fatal(format("FATAL COMPILATION ERROR: Compiler tried processing an invalid StatementNode either produced improperly by Parser, or Compiler should know the statement but hasnt been updated properly\n COMPILER ID: {}\n", pm_CompiledThreadID), "Compiler");
            return EXIT_FAILURE;
        }
    }

    fp_CompilationUnit->CompiledByteCode.push_back(BJ_OP::HALT); //indicate proper exit

    BinaryCodec::EncodeInt<int64_t>(fp_CompilationUnit->CompiledByteCode, 0);

    return BONGO_OK;
}
}//namespace BongoJam