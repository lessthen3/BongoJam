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
#include "runtime/Interpreter.h"
#include "compiler/Linker.h"
#include "Serializer.h"
#include "compiler/CompilerThreadPool.h"

///STL
#include <array>
#include <assert.h>

//"Assertion: %s @ %s:%d (pid:%d)", #x, __FILE__, __LINE__, Platform::GetProcessID()
#define BONGO_ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

#define BONGO_ASSERT(x) \
do {                                                     \
   if (!(x)) \
   {                                           \
       BongoJam::PrintError("Assertion Failed!"); \
       exit(0); \
   } \
} while (false)

namespace BongoJam
{
    constexpr int BONGO_NO_MAIN_FOUND = 2000;
    constexpr int BONGO_MULTIPLE_MAINS_FOUND = 2001;

    constexpr int NO_ARGUMENT_PROVIDED = -1001;
    constexpr int UNKNOWN_OR_INCOMPLETE_ARGUMENT = -1002;
    constexpr int INVALID_SET_ARGUMENT = -1003;
    constexpr int INVALID_SCRIPT_TARGET = -1004;
    constexpr int SCRIPT_DOES_NOT_EXIST = -1005;

    constexpr int NO_SCRIPTS_GIVEN = -1006;
    constexpr int NO_OUTPUT_FILE_NAME_GIVEN = -1007;

    constexpr int BONGO_FAILED_TO_LOAD_PROJECT = -1008;

    constexpr int BONGO_NO_BUILD_TYPE_SPECIFIED = -1009;
    constexpr int BONGO_NO_OUTPUT_TYPE_SPECIFIED = -1010;

    struct BongoConfigs
    {
        BongoConfigs() = default;
        ~BongoConfigs() = default;

        string m_LogOutputDirectory = "./logs";

        Logger::LogLevel pm_ActiveLogMask = Logger::LogLevel::ALL_LOGS;

        uint64_t m_MaximumAllowedThreads = 1;

        void
            PrintConfigsToConsole()
            const
        {
            cout
                << "\n\n" //XXX: used for spacing messages from CLI call
                << CreateColouredText("Script File Path: ", Colours::BrightYellow)
                //<< CreateColouredText(m_ScriptFilePath, Colours::BrightCyan)
                << "\n"
                << CreateColouredText("Output File Name: ", Colours::BrightYellow)
                //<< CreateColouredText(m_OutputFileName + ".bongo", Colours::BrightCyan)
                << "\n"
                << CreateColouredText("Output Directory: ", Colours::BrightYellow)
                //<< CreateColouredText(m_BongoFileOutputDirectory, Colours::BrightCyan)
                << "\n"
                << CreateColouredText("Log Output Directory: ", Colours::BrightYellow)
                << CreateColouredText(m_LogOutputDirectory, Colours::BrightCyan)
                << "\n\n"
                ;
        }

        void
            CreateDefaultConfigs()
            const
        {
            ofstream f_Configs("configs.ini");

            if (not f_Configs)
            {
                PrintError("Failed to create default settings file.");
                return;
            }

            //string f_IsDebug = (m_IsDebugMode ? "true" : "false");

            f_Configs << "[Settings]\n";
            //f_Configs << "OutputFileName=" + m_OutputFileName + "\n";
            //f_Configs << "OutputDirectory=" + m_BongoFileOutputDirectory + "\n";
            //f_Configs << "DebugMode= " + f_IsDebug + "\n";
            f_Configs << "LogOutputDirectory= " + m_LogOutputDirectory + "\n";

            f_Configs.close();
        }

        void
            WriteToConfigs()
            const
        {
            ofstream f_Configs("configs.ini");

            if (not f_Configs)
            {
                PrintError("Failed to open config file for writing.");
                return;
            }

            //string f_IsDebug = (m_IsDebugMode ? "true" : "false");

            f_Configs << "[Settings]\n";
            //f_Configs << "OutputFileName=" + m_OutputFileName + "\n";
            //f_Configs << "OutputDirectory=" + m_BongoFileOutputDirectory + "\n";
            //f_Configs << "DebugMode= " + f_IsDebug + "\n";
            f_Configs << "LogOutputDirectory= " + m_LogOutputDirectory + "\n";

            f_Configs.close();
        }

        map<string, string>
            ReadConfigs()
            const
        {
            map<string, string> f_Configs;
            ifstream f_ConfigFile("configs.ini");

            if (not f_ConfigFile)
            {
                PrintError("Failed to open config file for reading.", Colours::Magenta);
                return f_Configs;
            }

            string f_CurrentLine; //used for tracking the current line of the ini

            while (getline(f_ConfigFile, f_CurrentLine))
            {
                // Remove comments
                size_t f_CommentPosition = f_CurrentLine.find('#');

                if (f_CommentPosition != string::npos)
                {
                    f_CurrentLine = f_CurrentLine.substr(0, f_CommentPosition);
                }

                // Remove spaces at the beginning
                f_CurrentLine.erase(0, f_CurrentLine.find_first_not_of(" \t"));

                // Ignore empty lines
                if (f_CurrentLine.empty() or f_CurrentLine[0] == ';' or f_CurrentLine[0] == '#' or f_CurrentLine[0] == '[')
                {
                    continue;
                }

                size_t delimiterPos = f_CurrentLine.find('=');
                string key = f_CurrentLine.substr(0, delimiterPos);
                string value = f_CurrentLine.substr(delimiterPos + 1);

                // Remove spaces around the key and value
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                f_Configs[key] = value;
            }

            f_ConfigFile.close();
            return f_Configs;
        }

        bool
            CreateProfile()
        {

            return true;
        }

        bool
            LoadProfile(const string& fp_ProfileName)
        {

            return true;
        }
    };

    struct BongoScriptFile
    {
        string ScriptName;
        string ScriptFilePath;

        SERIALIZABLE_FIELDS(ScriptName, ScriptFilePath)
    };

    struct CompilerConfigs //keeps track of compiler settings and all source paths
    {
        string MainFilePath;
        // Name : Path
        vector<BongoScriptFile> BongoScripts;

        string OutputFileName = "rawr_uwu"; //everything gets stiched together into a single IR executable or .bcu (bongo compiled unit aka a static lib + name table for includes and optional commenting)
        string OutputDirectory = "./";

        uint64_t CompilerFlags = BongoCompilerFlags::DEFAULT;

        SERIALIZABLE_FIELDS(MainFilePath, BongoScripts, OutputFileName, OutputDirectory, CompilerFlags)
    };

    struct BongoProject //serialized to .bsproj file when building
    {
        string ProjectName = "OwO";
        string ProjectVersion = "0.0.1";

        string BongoJamVersion = "0.0.1";
        string BongoCompilerVersion = "0.0.1";

        CompilerConfigs LastUsedCompilerConfigs;

        SERIALIZABLE_FIELDS(ProjectName, ProjectVersion, BongoJamVersion, BongoCompilerVersion, LastUsedCompilerConfigs)
    };

    struct CurrentBongoProject //used by bongomanager during runtime
    {
        BongoProject Project;
        filesystem::path pm_CurrentBongoProjectFilePath;
    };
}

namespace BongoJam {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<size_t pm_MaximumAllowedThreads>
    class BongoManager
    {
        static_assert(pm_MaximumAllowedThreads > 0, "Thread count must be greater than 0");

        //////////////////////////////////////////////
        // Constructor and Destructor
        //////////////////////////////////////////////
    public:
        BongoManager()
        {
            bongo_logger = make_unique<Logger>();
            bongo_logger->Initialize(DEFAULT_LOG_OUTPUT_DIRECTORY, "BongoManager", DEFAULT_LOG_LEVEL_FILTER);
            bongo_logger->Debug("uwu", "BongoManager");

            pm_Linker = make_unique<BongoLinker>();
            pm_Interpreter = make_unique<BongoJamInterpreter>();
        }

        ~BongoManager()
        {
            pm_CompilerThreadPool.Shutdown();
        }

        BongoManager(const BongoManager&) = delete;
        BongoManager& operator=(const BongoManager&) = delete;
        BongoManager(BongoManager&&) = delete;
        BongoManager& operator=(BongoManager&&) = delete;

    public:
        BongoConfigs Configs; //persistent options, bongomanager is used for per session or project basis

    private:
        unique_ptr<Logger> bongo_logger = nullptr;

        unique_ptr<BongoLinker> pm_Linker = nullptr;
        unique_ptr<BongoJamInterpreter> pm_Interpreter = nullptr;

        vector<BongoScriptUnit> pm_CurrentProjectSources;
        vector<BongoScriptUnit> pm_FoundMains;

        Serializer pm_Serializer;

        CurrentBongoProject pm_CurrentProject;

        CompilerThreadPool<pm_MaximumAllowedThreads> pm_CompilerThreadPool;

    public:

        int
            LoadProject(const string& fp_ProjectDirectory, CompilerConfigs& fp_CompilerConfigs)
        {
            if (not pm_Serializer.FromJSON(pm_CurrentProject.Project, fp_ProjectDirectory, bongo_logger.get()))
            {

                return BONGO_FAILED_TO_LOAD_PROJECT;
            }

            fp_CompilerConfigs = pm_CurrentProject.Project.LastUsedCompilerConfigs;

            return BONGO_OK;
        }

        int
            CreateNewProject()
        {

            return BONGO_OK;
        }

        int
            AddScriptsToExistingProject()
        {

            return BONGO_OK;
        }

        char** 
            StringVectorToCharArray(const vector<string>& fp_StringVector) 
        {
            // Allocate memory for the array of char* pointers
            char** f_CharArray = new char* [fp_StringVector.size() + 1]; // +1 for the null terminator

            // Iterate through the vector and convert each string
            for (size_t _i = 0; _i < fp_StringVector.size(); ++_i)
            {
                const string& s = fp_StringVector[_i];
                // Allocate memory for the C-style string (including null terminator)
                f_CharArray[_i] = new char[s.length() + 1];
                // Copy the string content
                strcpy(f_CharArray[_i], s.c_str());
            }

            // Null-terminate the array of pointers
            f_CharArray[fp_StringVector.size()] = nullptr;

            return f_CharArray;
        }

        void FreeCharArray(char** charArray) 
        {
            if (charArray == nullptr) 
            {
                return;
            }
            // Free each individual C-style string
            for (size_t i = 0; charArray[i] != nullptr; ++i)
            {
                delete[] charArray[i];
            }

            // Free the array of pointers itself
            delete[] charArray;
        }

        int
            ParseArguments(const vector<string>& fp_Args) //for easier use from C++
        {
            char** f_CharArray = StringVectorToCharArray(fp_Args);

            int result = ParseArguments(fp_Args.size(), f_CharArray);

            FreeCharArray(f_CharArray);

            return result;
        }

        int
            ParseArguments(int fp_ArgCount, char* fp_ArgVector[])
        {
            if (fp_ArgCount < 2)
            {
                PrintError("No arguments provided. Use -h or --help for usage information.", Colours::Magenta);
                return NO_ARGUMENT_PROVIDED;
            }

            bool f_IsOutputTypeSpecified = false;
            bool f_IsBuildTypeSpecified = false;
            bool f_IsCompileRun = false;

            for (int _i = 1; _i < fp_ArgCount; ++_i) //check for args that could fuck up other processes
            {
                string f_CompilerArg = fp_ArgVector[_i];

                //////////////////// Valid First Args ////////////////////

                if (f_CompilerArg == "-h" or f_CompilerArg == "--help")
                {
                    DisplayHelp();
                    return BONGO_OK;
                }
                else if (f_CompilerArg == "--version")
                {
                    cout  
                        << CreateColouredText("Current BongoJam Compiler Version: ", Colours::BrightMagenta) << CreateColouredText(BONGO_COMPILER_VERSION, Colours::BrightCyan) << "\n"
                        << CreateColouredText("Current BongoJam Compiler Version: ", Colours::BrightMagenta) << CreateColouredText(BONGO_RUNTIME_VERSION, Colours::BrightCyan) << "\n"
                    ;

                    return BONGO_OK;
                }
                else if (f_CompilerArg == "-r")
                {
                    cout
                        << CreateColouredText("Current BongoJam Compiler Version: ", Colours::BrightMagenta) << CreateColouredText(BONGO_COMPILER_VERSION, Colours::BrightCyan) << "\n"
                        << CreateColouredText("Current BongoJam Compiler Version: ", Colours::BrightMagenta) << CreateColouredText(BONGO_RUNTIME_VERSION, Colours::BrightCyan) << "\n"
                        ;

                    return BONGO_OK;
                }
            }

            for (int _i = 1; _i < fp_ArgCount; ++_i)
            {
                string f_CompilerArg = fp_ArgVector[_i];

                if (f_CompilerArg == "-cwd")
                {
                    //////////////////// find main function and scripts path - check ////////////////////

                    int result = ScanDirectoryRecursivelyForScripts(filesystem::current_path());

                    if (result != BONGO_OK)
                    {
                        return result;
                    }

                    //////////////////// unique main script found, and all scripts for project, time to start compilation of each script -> CompilationUnit ////////////////////

                    pm_CurrentProject.Project.LastUsedCompilerConfigs.OutputFileName = "rawr_uwu"; //quick extension substitution (later at compile time)
                }
                else if (f_CompilerArg == "-sp") // sp = specified, for scripts specified individually
                {
                    _i++; //advance forward onemore step

                    if (not (_i < fp_ArgCount)) //check for bounds
                    {
                        bongo_logger->Fatal("No files input for compiler after '-sp' flag was used, try -cwd if you're looking for automatic script searching", "ParseArguments");
                        return NO_SCRIPTS_GIVEN;
                    }

                    f_CompilerArg = string(fp_ArgVector[_i]);
                    size_t f_Dot = f_CompilerArg.rfind('.');

                    if (f_Dot != string::npos and f_CompilerArg.substr(f_Dot) == ".bsproj")
                    {
                        pm_CurrentProject.Project.LastUsedCompilerConfigs.BongoScripts.clear(); //just in case some dumbass does "script script proj script"
                        LoadProject(f_CompilerArg, pm_CurrentProject.Project.LastUsedCompilerConfigs); //assuming the proj file is passed as ../../somefolder/name.bsproj i dont think thats a bold assumption

                        continue;
                    }

                    if (f_CompilerArg != "[")
                    {
                        bongo_logger->Fatal("No files input for compiler after '-sp' flag was used, try -cwd if you're looking for automatic script searching", "ParseArguments");
                        return NO_SCRIPTS_GIVEN;
                    }

                    while (_i + 1 < fp_ArgCount and f_CompilerArg != "]")
                    {
                        ++_i;
                        f_CompilerArg = string(fp_ArgVector[_i]);
                        BongoScriptFile f_Script;

                        f_Script.ScriptName = f_CompilerArg;
                        size_t f_Dot = f_Script.ScriptName.rfind('.');

                        f_Script.ScriptFilePath = "./" + f_Script.ScriptName; // we add before hand because we want a bj here :^)

                        if (not filesystem::exists(f_Script.ScriptName))
                        {
                            PrintError("No script with name: " + f_Script.ScriptName + " found in top-level directory", Colours::Magenta);
                            return SCRIPT_DOES_NOT_EXIST;
                        }

                        if (f_Dot != string::npos and f_Script.ScriptName.substr(f_Dot) == ".bj")
                        {
                            f_Script.ScriptName = f_Script.ScriptName.substr(0, f_Dot);
                        }
                        else
                        {
                            PrintError("Invalid script target found, please only try to compile .bj files only", Colours::Magenta);
                            return INVALID_SCRIPT_TARGET;
                        }

                        pm_CurrentProject.Project.LastUsedCompilerConfigs.BongoScripts.push_back(f_Script); //add script to current compiler configs
                    }

                    if (f_CompilerArg != "]")
                    {
                        bongo_logger->Fatal("No ']' found terminating list of scripts after -sp command", "ParseArguments");
                        return NO_SCRIPTS_GIVEN;
                    }

                }//
                else if (f_CompilerArg == "--build_exe")
                {
                    if (pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_DYNAMIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE OPTION SELECTED: tried to pass --build_exe when build_dynamic was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else if (pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_STATIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE OPTION SELECTED: tried to pass --build_exe when build_static was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else if (f_IsCompileRun)
                    {
                        bongo_logger->Warning("INCOMPATIBLE OPTION SELECTED: tried to pass --build_exe when --compilerun was already selected, assuming --compilerun was intended", "ParseArguments");
                        continue;
                    }
                    else
                    {
                        pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags |= BongoCompilerFlags::BUILD_EXECUTABLE;
                        f_IsOutputTypeSpecified = true;
                    }
                }
                //////////////////// Only Valid if Script put in ////////////////////
                else if (f_CompilerArg == "-o" )
                {
                    if (not (_i + 1 < fp_ArgCount)) //check for bounds
                    {
                        bongo_logger->Fatal("No output file name was input after '-o' flag was used, please input a valid string after [-o] is used", "ParseArguments");
                        return NO_OUTPUT_FILE_NAME_GIVEN;
                    }

                    pm_CurrentProject.Project.LastUsedCompilerConfigs.OutputFileName = fp_ArgVector[++_i];  // Increment `_i` to skip the next argument, which is the filename
                }
                else if (f_CompilerArg == "--debug")
                {
                    if (f_IsBuildTypeSpecified)
                    {
                        bongo_logger->Warning("Build type already specified, using first mention and ignoring any repeated mention", "ParseArguments");
                        continue;
                    }

                    pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags |= BongoCompilerFlags::DEBUG;
                    f_IsBuildTypeSpecified = true;
                }
                else if (f_CompilerArg == "--release")
                {
                    if (f_IsBuildTypeSpecified)
                    {
                        bongo_logger->Warning("Build type already specified, using first mention and ignoring any repeated mention", "ParseArguments");
                        continue;
                    }

                    pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags |= BongoCompilerFlags::RELEASE;
                    f_IsBuildTypeSpecified = true;
                }
                else if (f_CompilerArg == "--compilerun")
                {
                    if (pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_DYNAMIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE BUILD TYPE SELECTED: tried to pass --compilerun when build_dynamic was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else if (pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_STATIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE BUILD TYPE SELECTED: tried to pass --compilerun when build_static was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else
                    {
                        f_IsCompileRun = true;
                        pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags = pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags | BongoCompilerFlags::BUILD_EXECUTABLE;
                        f_IsOutputTypeSpecified = true;
                    }
                }
                else if (f_CompilerArg == "--build_static")
                {
                    if (f_IsOutputTypeSpecified)
                    {
                        bongo_logger->Warning("Build output type already specified, using first mention and ignoring any repeated mention", "ParseArguments");
                        continue;
                    }
                    else if (f_IsCompileRun)
                    {
                        bongo_logger->Warning("INCOMPATIBLE BUILD TYPE SELECTED: tried to pass build_static when --compilerun was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }

                    pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags |= BongoCompilerFlags::BUILD_STATIC_LIBRARY;
                    f_IsOutputTypeSpecified = true;
                }
                else if (f_CompilerArg == "--build_dynamic")
                {
                    if (f_IsOutputTypeSpecified)
                    {
                        bongo_logger->Warning("Build output type already specified, using first mention and ignoring any repeated mention", "ParseArguments");
                        continue;
                    }
                    else if (f_IsCompileRun)
                    {
                        bongo_logger->Warning("INCOMPATIBLE BUILD TYPE SELECTED: tried to pass build_dynamic when --compilerun was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }

                    pm_CurrentProject.Project.LastUsedCompilerConfigs.CompilerFlags |= BongoCompilerFlags::BUILD_DYNAMIC_LIBRARY;

                    f_IsOutputTypeSpecified = true;
                }
                else
                {
                    PrintError("Unknown or incomplete argument provided: " + f_CompilerArg, Colours::Magenta);
                    return UNKNOWN_OR_INCOMPLETE_ARGUMENT;
                }
            }

            if (not f_IsOutputTypeSpecified)
            {
                bongo_logger->Error("No build output type detected, please specify whether you are building an executable or library. nothing was done.", "ParseArguments");
                return BONGO_NO_OUTPUT_TYPE_SPECIFIED;
            }
            else if (not f_IsBuildTypeSpecified)
            {
                bongo_logger->Error("No build type detected, please specify whether you are building in debug or release. nothing was done.", "ParseArguments");
                return BONGO_NO_BUILD_TYPE_SPECIFIED;
            }

            return RunCommands(pm_CurrentProject.Project.LastUsedCompilerConfigs, f_IsCompileRun); //if its not BONGO_OK or whatever then rip uwu cant do anything ab it at this point OwO >O<
        }

        int
            RunCommands(const CompilerConfigs& fp_CompilerConfigs, bool fp_IsCompileRun)
        {
            if (StartCompilationOfProject(fp_CompilerConfigs) != BONGO_OK)
            {
                bongo_logger->Error("Compilation Failed! nothing was done.", "RunCommands");
                return BONGO_COMPILATION_FAILED;
            }

            if (fp_IsCompileRun)
            {
                if(fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::DEBUG)
                {
                    const auto BongoJam_Timer_Start = chrono::high_resolution_clock::now();
                    pm_Interpreter->BongoTime(fp_CompilerConfigs.OutputDirectory + "/" + fp_CompilerConfigs.OutputFileName + ".bongo");
                    const auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

                    const auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);
                }
                else
                {
                    pm_Interpreter->BongoTime(fp_CompilerConfigs.OutputDirectory + "/" + fp_CompilerConfigs.OutputFileName + ".bongo");
                }
            }

            return EXIT_SUCCESS;
        }

        void
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
            ScanDirectoryRecursivelyForScripts(const filesystem::path& fp_RootPath)
        {
            for (const auto& entry : filesystem::recursive_directory_iterator(fp_RootPath))
            {
                string f_FileName = entry.path().filename().string();
                size_t f_Dot = f_FileName.rfind('.');

                if (entry.is_regular_file() and f_Dot != string::npos)
                {
                    if(f_FileName == "main.bj")
                    {
                        bongo_logger->Debug(format("main.bj found at : '{}'", entry.path().string()), "BongoManager");
                        pm_FoundMains.emplace_back(entry.path());
                    }
                    else if (f_FileName.substr(f_Dot) == ".bj")
                    {
                        pm_CurrentProjectSources.emplace_back(entry.path());
                    }
                    else if (f_FileName.substr(f_Dot) == ".bsproj") //only assumes one .bsproj in the directory atm will just use the last one found as the bsproj currently
                    {
                        pm_CurrentProject.pm_CurrentBongoProjectFilePath = entry;
                    }
                }
            }

            if (pm_FoundMains.empty())
            {
                bongo_logger->Fatal("No main.bj script found anywhere in project directory!", "BongoManager");
                return BONGO_NO_MAIN_FOUND;
            }

            if (pm_FoundMains.size() > 1)
            {
                bongo_logger->Fatal("Multiple main.bj scripts found! Please make sure only one exists.", "BongoManager");

                for (const auto& lv_ScriptUnit : pm_FoundMains)
                {
                    bongo_logger->Error(format(" ---  {}", lv_ScriptUnit.FilePath.string()), "BongoManager");
                }

                return BONGO_MULTIPLE_MAINS_FOUND;
            }

            return BONGO_OK;
        }

        int
            StartCompilationOfProject(const CompilerConfigs& fp_CompilerConfigs)
        {
            pm_CompilerThreadPool.BONGO_COMPILE_SUCCESS = true; //set flag to true and worker threads will set to false if failed uwu

            //read file paths into a job queue

            pm_CompilerThreadPool.EnqueueTask({ pm_FoundMains[0].FilePath.string() , pm_FoundMains[0].CompiledUnit.get() });

            for (int _i = 0; _i < pm_CurrentProjectSources.size(); ++_i)
            {
                const string f_SourcePath = pm_CurrentProjectSources[_i].FilePath.string();
                CompilationUnit* f_CompilationUnit = pm_CurrentProjectSources[_i].CompiledUnit.get();

                pm_CompilerThreadPool.EnqueueTask({ f_SourcePath , f_CompilationUnit });
            }

            pm_CompilerThreadPool.WaitUntilAllTasksComplete();

            if (not pm_CompilerThreadPool.BONGO_COMPILE_SUCCESS)
            {

                return EXIT_FAILURE;
            }

            pm_CurrentProjectSources.push_back(move(pm_FoundMains[0])); //put entry point as last item

            //////////////////// every script was validated and compiled into a CompilationUnit, Linker time baby ////////////////////
            // also need to find precompiled CompilationUnits via configs for external deps
            //link together compilationunits, assuming everything was checked properly, every script unit should have a corresponding compilationunit attached to it uwu
            //run linker to resolve symbols, and return error if found fingys cwossed >w<  

            vector<uint8_t> f_FullBongoProgram;
            pm_Linker->LinkCompilationUnits(move(pm_CurrentProjectSources), f_FullBongoProgram);
            
            //////////////////// Write fully assembled BongoJam program that is ready to be run >O< ////////////////////
            if (not pm_Linker->WriteBytecodeToFile(f_FullBongoProgram, fp_CompilerConfigs.OutputDirectory, fp_CompilerConfigs.OutputFileName))
            {

                return EXIT_FAILURE;
            }

            //after successfully writing bytecode to a file return BONGO_OK
            return BONGO_OK;
        }

#ifdef BONGO_DEBUG
        int
            RunTest(const string& fp_ScriptPath)
        {
            pm_CompilerThreadPool.BONGO_COMPILE_SUCCESS = true; //set flag to true and worker threads will set to false if failed uwu

            BongoScriptUnit fp_TestScript;

            //read file paths into a job queue

            pm_CompilerThreadPool.EnqueueTask({ fp_ScriptPath , fp_TestScript.CompiledUnit.get() });

            pm_CompilerThreadPool.WaitUntilAllTasksComplete();

            if (not pm_CompilerThreadPool.BONGO_COMPILE_SUCCESS)
            {

                return EXIT_FAILURE;
            }

            pm_CurrentProjectSources.push_back(move(fp_TestScript)); //put entry point as last item

            //////////////////// every script was validated and compiled into a CompilationUnit, Linker time baby ////////////////////
            // also need to find precompiled CompilationUnits via configs for external deps
            //link together compilationunits, assuming everything was checked properly, every script unit should have a corresponding compilationunit attached to it uwu
            //run linker to resolve symbols, and return error if found fingys cwossed >w<  

            vector<uint8_t> f_FullBongoProgram;
            pm_Linker->LinkCompilationUnits(move(pm_CurrentProjectSources), f_FullBongoProgram);

            //////////////////// Write fully assembled BongoJam program that is ready to be run >O< ////////////////////
            if (not pm_Linker->WriteBytecodeToFile(f_FullBongoProgram, "./", "rawr_uwu"))
            {

                return EXIT_FAILURE;
            }

            //after successfully writing bytecode to a file return BONGO_OK
            return pm_Interpreter->BongoTime("./rawr_uwu.bongo");
        }
#endif
    };
}//namespace BongoJam