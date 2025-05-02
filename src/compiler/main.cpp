/*******************************************************************
 *                                        BongoJam Script v0.0.6
 *                           Created by Ranyodh Mandur - � 2024
 *
 *                         Licensed under the MIT License (MIT).
 *                  For more details, see the LICENSE file or visit:
 *                        https://opensource.org/licenses/MIT
 *
 *  BongoJam is an open-source scripting language compiler and interpreter
 *              primarily intended for embedding within game engines.
********************************************************************/
#include "../../include/ConfigManager.h"
#include "../../include/compiler/Compiler.h"
#include "../../include/runtime/Interpreter.h"

using namespace BongoJam;

constexpr int NO_ARGUMENT_PROVIDED = -1001;
constexpr int UNKNOWN_OR_INCOMPLETE_ARGUMENT = -1002;
constexpr int INVALID_SET_ARGUMENT = -1003;
constexpr int INVALID_SCRIPT_TARGET = -1004;
constexpr int SCRIPT_DOES_NOT_EXIST = -1005;


static void
    DisplayHelp() 
{
    cout 
        << CreateColouredText("{Usage}: bongo <file> [options]...\n", Colours::BrightMagenta)

        << CreateColouredText("Compiler Options:\n  ", Colours::BrightYellow) //oh it's because i put two spaces after each \n lmao
        << CreateColouredText("  -o <filename>\t{Usage}:Set output filename\n  ", Colours::Cyan) //this needs an extra two spaces at the beginning for god knows what reason ?_? !!
        << CreateColouredText("  -d <directory>\t{Usage}:Set output directory\n  ", Colours::Cyan)
           
        << CreateColouredText("  --compile-run\t{Usage}:If used, the script will be compiled and ran immediately\n  ", Colours::Cyan)
        << CreateColouredText("  --debug\t{Usage}:Compile in debug mode\n  ", Colours::Cyan)
        << CreateColouredText("  --pedantic\t{Usage}:Compile with all warnings turned on\n  ", Colours::Cyan)

        << CreateColouredText("  --clear-logs\t{Usage}: Clears Desired Log Files\n", Colours::Cyan)
            << CreateColouredText("\t  [option 1] LOG_LEVEL_MINOR - LOG_LEVEL_MAJOR\n", Colours::BrightGreen)
            << CreateColouredText("\t  [option 2] LOG_LEVEL_1, LOG_LEVEL_2 . . .\n  ", Colours::BrightGreen)

        << CreateColouredText("  --set LOG_LEVEL_FILTER\t{Usage}: Filters Log Output\n  ", Colours::Cyan) //disable/enable internal logs, and set the min and max log level, one arg is min, two args is both
            << CreateColouredText("\t  [option 1] LOG_LEVEL_MINOR - LOG_LEVEL_MAJOR\n", Colours::BrightGreen)
            << CreateColouredText("\t  [option 2] LOG_LEVEL_1, LOG_LEVEL_2 . . .\n  ", Colours::BrightGreen)

        << CreateColouredText("  --set LOG_OUTPUT_DIRECTORY <directory>\t{Usage}:Sets Working Log Output Directory\n  ", Colours::Cyan)
        << CreateColouredText("  --set DEFAULT_OUTPUT_DIRECTORY <directory>\t{Usage}:Sets Default Log Output Directory\n  ", Colours::Cyan)

        << CreateColouredText("  -h, --help\t{Usage}:Display this help and exit\n  ", Colours::Cyan)
        << CreateColouredText("  --version\t{Usage}:Get the currently installed compiler version\n  ", Colours::Cyan)
    ;
}

int 
    main(int fp_ArgCount, char* fp_ArgVector[])
{
    //Enable ANSI colour codes for windows console grumble grumble
    #if defined(_WIN32) or defined(_WIN64)
        EnableColors();
    #endif

    ConfigManager Configs; //initializes with default settings

    BongoCompiler* BongoJamCompiler = new BongoCompiler();

    if (fp_ArgCount < 2) 
    {
        PrintError("No arguments provided. Use -h or --help for usage information.", Colours::Magenta);
        return NO_ARGUMENT_PROVIDED;
    }

    vector<string> Desired_Logs;

    for (int _i = 1; _i < fp_ArgCount; ++_i) 
    {
        string Compiler_Arg = fp_ArgVector[_i];

        //////////////////// Valid First Args ////////////////////

        if (Compiler_Arg == "-h" or Compiler_Arg == "--help") 
        {
            DisplayHelp();
            return EXIT_SUCCESS;
        } 
        else if (Compiler_Arg == "--version")
        {
            cout << CreateColouredText("Current BongoJam Compiler Version: ", Colours::BrightMagenta) << CreateColouredText(Configs.m_BongoJamVersion, Colours::BrightCyan) << "\n";
            return EXIT_SUCCESS;
        }
        else if (_i == 1)
        {
            string f_ScriptName(Compiler_Arg);
            size_t f_Dot = f_ScriptName.rfind('.');

            Configs.m_ScriptFilePath = "./" + f_ScriptName; // we add before hand because we want a bj here :^)

            if (not filesystem::exists(f_ScriptName))
            {
                PrintError("No script with name: " + f_ScriptName + " found in top-level directory", Colours::Magenta);
                return SCRIPT_DOES_NOT_EXIST;
            }

            if (f_Dot != string::npos and f_ScriptName.substr(f_Dot) == ".bj")
            {
                Configs.m_ScriptName = f_ScriptName.substr(0, f_Dot);
            }
            else
            {
                PrintError("Invalid script target found, please only try to compile .bj files only", Colours::Magenta);
                return INVALID_SCRIPT_TARGET;
            }

            Configs.m_OutputFileName = Configs.m_ScriptName; //quick extension substitution (later at compile time)
        }

        //////////////////// Only Valid if Script put in ////////////////////

        else if (Compiler_Arg == "--debug") 
        {
            Configs.m_IsDebugMode = true;
        } 
        else if (Compiler_Arg == "-o" and _i + 1 < fp_ArgCount) 
        {
            Configs.m_OutputFileName = fp_ArgVector[++_i];  // Increment `_i` to skip the next argument, which is the filename
        }
        else if (Compiler_Arg == "--set")
        {
            _i++;
            Compiler_Arg = fp_ArgVector[_i];

            if (Compiler_Arg == "LOG_LEVEL_FILTER")
            {
                if (_i + 2 < fp_ArgCount and fp_ArgVector[_i + 2] == "-")
                {
                    //handle the log range wanted, in fashion level_minor - means from minor up, and -level_major is major and below
                }
                else
                {
                    //handle the listed logs
                }
            }
            else if (Compiler_Arg == "LOG_OUTPUT_DIRECTORY")
            {

            }
            else
            {
                PrintError("No valid arguments for --set were given. Use -h or --help for usage information.", Colours::Magenta);
                return INVALID_SET_ARGUMENT;
            }
        }
        else if (Compiler_Arg == "--compile-run")
        {
            Configs.m_IsCompileRun = true;
        }
        else 
        {
            PrintError("Unknown or incomplete argument provided: " + Compiler_Arg, Colours::Magenta);
            return UNKNOWN_OR_INCOMPLETE_ARGUMENT;
        }
    }

    unique_ptr<Logger> mf_BongoLogger = make_unique<Logger>();
    
    //Initialize logger for the compiler
    if (Configs.m_DesiredLogs)
    {
        mf_BongoLogger->Initialize("BongoLog", Configs.m_LogOutputDirectory, *Configs.m_DesiredLogs);
    }
    else
    {
        mf_BongoLogger->Initialize("BongoLog", Configs.m_LogOutputDirectory, Configs.m_LogLevelMinor, Configs.m_LogLevelMajor);
    }

    #ifdef _DEBUG
        mf_BongoLogger->LogAndPrint("Logger successfully initialized!", "main", Logger::LogLevel::Info);
    #endif

    if(Configs.m_IsDebugMode)
    {
        Configs.PrintConfigsToConsole();
    }

    if (Configs.m_IsCompileRun and Configs.m_IsDebugMode)
    {
        BongoJamCompiler->CompileProgram
        (
            Configs.m_ScriptFilePath, 
            Configs.m_BongoFileOutputDirectory, 
            Configs.m_OutputFileName, 
            mf_BongoLogger.get(), 
            Configs.m_IsDebugMode
        );

        BongoJamInterpreter* BongoJamRuntime = new BongoJamInterpreter();

        auto BongoJam_Timer_Start = chrono::high_resolution_clock::now();
        BongoJamRuntime->RunBongoScript(Configs.m_BongoFileOutputDirectory + "/" + Configs.m_OutputFileName + ".bongo");
        auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

        auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);

        // Output the time taken by bongojam
        cout 
            << CreateColouredText("\nTime taken by bongojam interpreter: ", Colours::BrightYellow) 
            << BongoJam_Runtime_Duration.count() 
            << CreateColouredText(" microseconds", Colours::BrightBlue) 
            << "\n\n\n";
        
        delete BongoJamRuntime; //probably should let the os handle cleaning up the heap alloc since its faster but w/e it feels wrong not to do this
        BongoJamRuntime = nullptr;
    }
    else if (Configs.m_IsCompileRun)
    {
        BongoJamCompiler->CompileProgram
        (
            Configs.m_ScriptFilePath, 
            Configs.m_BongoFileOutputDirectory, 
            Configs.m_OutputFileName, 
            mf_BongoLogger.get(), 
            Configs.m_IsDebugMode
        );

        BongoJamInterpreter* BongoJamRuntime = new BongoJamInterpreter();

        BongoJamRuntime->RunBongoScript(Configs.m_BongoFileOutputDirectory + "/" + Configs.m_OutputFileName);

        delete BongoJamRuntime; //probably should let the os handle cleaning up the heap alloc since its faster but w/e it feels wrong not to do this
        BongoJamRuntime = nullptr;
    }
    else
    {
        BongoJamCompiler->CompileProgram
        (
            Configs.m_ScriptFilePath, 
            Configs.m_BongoFileOutputDirectory, 
            Configs.m_OutputFileName, 
            mf_BongoLogger.get(), 
            Configs.m_IsDebugMode
        );
    }

    return EXIT_SUCCESS;
}