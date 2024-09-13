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
#include "../../include/compiler/ConfigManager.h"
#include "../../include/compiler/Compiler.h"
#include "../../include/runtime/Interpreter.h"

#include <llvm/CodeGen/Analysis.h>

using namespace std;
using namespace BongoJam;

constexpr int NO_ARGUMENT_PROVIDED = -1;
constexpr int UNKNOWN_OR_INCOMPLETE_ARGUMENT = -2;
constexpr int INVALID_SET_ARGUMENT = -3;
constexpr int INVALID_SCRIPT_TARGET = -4;
constexpr int SCRIPT_DOES_NOT_EXIST = -5;


static void
    DisplayHelp() 
{
    cout << CreateColouredText("{Usage}: bongo <file> [options]...\n", "bright magenta")

            << CreateColouredText("Compiler Options:\n  ", "bright yellow") //oh it's because i put two spaces after each \n lmao
            << CreateColouredText("  -o <filename>\t{Usage}:Set output filename\n  ", "cyan") //this needs an extra two spaces at the beginning for god knows what reason ?_? !!
            << CreateColouredText("  -d <directory>\t{Usage}:Set output directory\n  ", "cyan")
           
            << CreateColouredText("  --compile-run\t{Usage}:If used, the script will be compiled and ran immediately\n  ", "cyan")
            << CreateColouredText("  --debug\t{Usage}:Compile in debug mode\n  ", "cyan")
            << CreateColouredText("  --pedantic\t{Usage}:Compile with all warnings turned on\n  ", "cyan")

            << CreateColouredText("  --clear-logs\t{Usage}: Clears Desired Log Files\n", "cyan")
                << CreateColouredText("\t  [option 1] LOG_LEVEL_MINOR - LOG_LEVEL_MAJOR\n", "bright green")
                << CreateColouredText("\t  [option 2] LOG_LEVEL_1, LOG_LEVEL_2 . . .\n  ", "bright green")

            << CreateColouredText("  --set LOG_LEVEL_FILTER\t{Usage}: Filters Log Output\n  ", "cyan") //disable/enable internal logs, and set the min and max log level, one arg is min, two args is both
                << CreateColouredText("\t  [option 1] LOG_LEVEL_MINOR - LOG_LEVEL_MAJOR\n", "bright green")
                << CreateColouredText("\t  [option 2] LOG_LEVEL_1, LOG_LEVEL_2 . . .\n  ", "bright green")

            << CreateColouredText("  --set LOG_OUTPUT_DIRECTORY <directory>\t{Usage}:Sets Working Log Output Directory\n  ", "cyan")
            << CreateColouredText("  --set DEFAULT_OUTPUT_DIRECTORY <directory>\t{Usage}:Sets Default Log Output Directory\n  ", "cyan")

            << CreateColouredText("  -h, --help\t{Usage}:Display this help and exit\n  ", "cyan")
            << CreateColouredText("  --version\t{Usage}:Get the currently installed compiler version\n  ", "cyan")

    ;
}

static bool 
    FileExists(const string& fp_FileName) 
{
    filesystem::path f_FilePath(fp_FileName);
    return filesystem::exists(f_FilePath);
}
int 
    main(int fp_ArgCount, char* fp_ArgVector[])
{
    //Enable ANSI colour codes for windows console grumble grumble
    #if defined(_WIN32) || defined(_WIN64)
        EnableColors();
    #endif

    ConfigManager Configs; //initializes with default settings

    if (fp_ArgCount < 2) 
    {
        PrintError("No arguments provided. Use -h or --help for usage information.", "magenta");
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
            cout << CreateColouredText("Current BongoJam Compiler Version: ", "bright magenta") << CreateColouredText(Configs.m_BongoJamVersion, "bright cyan") << "\n";
            return EXIT_SUCCESS;
        }
        else if (_i == 1)
        {
            string f_ScriptName(Compiler_Arg);
            size_t f_Dot = f_ScriptName.rfind('.');

            Configs.m_ScriptFilePath = "./" + f_ScriptName; // we add before hand because we want a bj here :^)

            if (!FileExists(f_ScriptName))
            {
                PrintError("No script with name: " + f_ScriptName + " found in top-level directory", "magenta");
                return SCRIPT_DOES_NOT_EXIST;
            }

            if (f_Dot != string::npos and f_ScriptName.substr(f_Dot) == ".bj")
            {
                Configs.m_ScriptName = f_ScriptName.substr(0, f_Dot);
            }
            else
            {
                PrintError("Invalid script target found, please only try to compile .bj files only", "magenta");
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
                PrintError("No valid arguments for --set were given. Use -h or --help for usage information.", "magenta");
                return INVALID_SET_ARGUMENT;
            }
        }
        else if (Compiler_Arg == "--compile-run")
        {
            Configs.m_IsCompileRun = true;
        }
        else 
        {
            PrintError("Unknown or incomplete argument provided: " + Compiler_Arg, "magenta");
            return UNKNOWN_OR_INCOMPLETE_ARGUMENT;
        }
    }
    
    //Initialize logger for the compiler
    if (Configs.m_DesiredLogs)
    {
        LogManager::Logger().Initialize(Configs.m_LogOutputDirectory, Configs.m_DesiredLogs);
    }
    else
    {
        LogManager::Logger().Initialize(Configs.m_LogOutputDirectory, Configs.m_LogLevelMinor, Configs.m_LogLevelMajor);
    }

    LogManager::Logger().Log("Logger successfully initialized!", "main", "info");

    if(Configs.m_IsDebugMode)
    {
        cout << CreateColouredText("Output File Name: ", "bright yellow")
            << CreateColouredText(Configs.m_OutputFileName + ".bongo", "bright cyan")
            << "\n"
            << CreateColouredText("Output Directory: ", "bright yellow")
            << CreateColouredText(Configs.m_BongoFileOutputDirectory, "bright cyan")
            << "\n"
            << CreateColouredText("Log Output Directory: ", "bright yellow")
            << CreateColouredText(Configs.m_LogOutputDirectory, "bright cyan")
            << "\n\n"
            ;
    }

    if (Configs.m_IsCompileRun and Configs.m_IsDebugMode)
    {
        CompileProgram(Configs.m_ScriptFilePath, Configs.m_BongoFileOutputDirectory, Configs.m_OutputFileName, Configs.m_IsDebugMode);

        BongoJamInterpreter* BongoJamRuntime = new BongoJamInterpreter();

        auto BongoJam_Timer_Start = chrono::high_resolution_clock::now();
        BongoJamRuntime->RunBongoScript(Configs.m_BongoFileOutputDirectory + "/" + Configs.m_OutputFileName + ".bongo");
        auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

        auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);

        // Output the time taken by bongojam
        cout << CreateColouredText("\nTime taken by bongojam interpreter: ", "bright yellow") << BongoJam_Runtime_Duration.count() << CreateColouredText(" microseconds", "bright blue") << "\n";
        
        delete BongoJamRuntime; //probably should let the os handle cleaning up the heap alloc since its faster but w/e it feels wrong not to do this
        BongoJamRuntime = nullptr;
    }
    else if (Configs.m_IsCompileRun)
    {
        CompileProgram(Configs.m_ScriptFilePath, Configs.m_BongoFileOutputDirectory, Configs.m_OutputFileName, Configs.m_IsDebugMode);

        BongoJamInterpreter* BongoJamRuntime = new BongoJamInterpreter();

        BongoJamRuntime->RunBongoScript(Configs.m_BongoFileOutputDirectory + "/" + Configs.m_OutputFileName);

        delete BongoJamRuntime; //probably should let the os handle cleaning up the heap alloc since its faster but w/e it feels wrong not to do this
        BongoJamRuntime = nullptr;
    }
    else
    {
        Print("Script File Path: " + Configs.m_ScriptFilePath, "bright green");
        CompileProgram(Configs.m_ScriptFilePath, Configs.m_BongoFileOutputDirectory, Configs.m_OutputFileName, Configs.m_IsDebugMode);
    }

    return EXIT_SUCCESS;
}