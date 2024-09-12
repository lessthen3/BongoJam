#include "../../include/runtime/Interpreter.h"

using namespace std;
using namespace BongoJam;

static void 
    DisplayHelp() 
{
    cout << CreateColouredText("Usage: bongo [options] file...\n", "bright magenta")
            << CreateColouredText("Options:\n", "bright yellow")
            << CreateColouredText("  --debug         Enable debug mode\n", "cyan") //run the interpreter in debug mode
            << CreateColouredText("  -set SHOULD_LOG         Enable debug mode\n", "cyan") //disable/enable internal logs
            << CreateColouredText("  -set Log_Directory         Enable debug mode\n", "cyan")
            << CreateColouredText("  -h, --help      Display this help and exit\n", "cyan")
    ;
}

int 
    main(int fp_ArgCount, char* fp_ArgVector)
{
    //Initialize logger for the interpreter
    LogManager::Logger().Initialize("../logs");

        //Enable ANSI colour codes for windows console grumble grumble
    #if defined(_WIN32) || defined(_WIN64)
        EnableColors();
    #endif

        BongoJamInterpreter* Interpreter = new BongoJamInterpreter();

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

    //cout << "Hello World!" << "\n";
    return 0;
}