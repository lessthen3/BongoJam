#include <iostream>
#include <thread>
#include <chrono>

#include "../include/Interpreter.h"
#include "../include/LogManager.h"

#include "../include/Compiler.h"

using namespace std;
using namespace BongoJam;

int main()
{
    EnableColors();
    LogManager::MainLogger().Initialize("../logs", "MainLogger");
    //LogManager::MainLogger().Debug(("MainLogger successfully initialized!"), "main");

    BongoInterpreter* Interpreter = new BongoInterpreter();

    CompileProgram("../bj_scripts/HelloWorld.bj", "../bj_bytecode");

    auto BongoJam_Timer_Start = chrono::high_resolution_clock::now(); 
    Interpreter->RunBongoScript("../bj_bytecode/UwU.bongo");
    auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

    auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);

    // Output the time taken by bongojam
    cout << CreateColouredText("Time taken by bongojam interpreter: ", "bright yellow") << BongoJam_Runtime_Duration.count() << CreateColouredText(" microseconds", "bright blue") << "\n";

    delete Interpreter;
    Interpreter = nullptr;
    
    auto Cpp_Timer_Start = chrono::high_resolution_clock::now();
    cout << " \t Hello  \t World! \n \n \n" << "\n";
    cout << "Who's in paris?" << "\n";
    auto Cpp_Timer_Stop = chrono::high_resolution_clock::now();

    auto Cpp_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(Cpp_Timer_Start - Cpp_Timer_Stop);
    
    // Output the time taken by cpp
    cout << CreateColouredText("Time taken by cpp: ", "bright magenta") << Cpp_Runtime_Duration.count() << CreateColouredText(" microseconds", "bright blue") << "\n";

    return 0;
}