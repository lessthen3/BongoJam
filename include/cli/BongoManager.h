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

}

namespace BongoJam {

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

    struct CompilerConfigs
    {
        string MainFilePath;
        // Name : Path
        vector<BongoScriptFile> BongoScripts;

        string OutputFileName = "rawr_uwu"; //everything gets stiched together into a single IR executable or .bcu (bongo compiled unit aka a static lib + name table for includes and optional commenting)
        string OutputDirectory = "./";

        uint64_t CompilerFlags = BongoCompilerFlags::DEFAULT;

        SERIALIZABLE_FIELDS(MainFilePath, BongoScripts, OutputFileName, OutputDirectory, CompilerFlags)
    };

    struct BongoProject
    {
        string ProjectName = "OwO";

        string Version = "0.0.1";
        string BongoJamVersion = "0.0.1";

        CompilerConfigs LastUsedCompilerConfigs;

        SERIALIZABLE_FIELDS(Version, BongoJamVersion, LastUsedCompilerConfigs)
    };

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

            pm_BongoRuntimeVersion = pm_Interpreter->BONGO_VERSION;
        }

        ~BongoManager() = default;

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

        string pm_BongoRuntimeVersion;
        string pm_BongoCompilerVersion;

        Serializer pm_Serializer;

        BongoProject pm_CurrentBongoProject;

        CompilerThreadPool<pm_MaximumAllowedThreads> pm_CompilerThreadPool;

    public:

        int
            LoadProject(const string& fp_ProjectDirectory, CompilerConfigs& fp_CompilerConfigs)
        {
            if (not pm_Serializer.FromJSON(pm_CurrentBongoProject, fp_ProjectDirectory, bongo_logger.get()))
            {

                return BONGO_FAILED_TO_LOAD_PROJECT;
            }

            fp_CompilerConfigs = pm_CurrentBongoProject.LastUsedCompilerConfigs;

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

        int
            ParseArguments(int fp_ArgCount, char* fp_ArgVector[], CompilerConfigs& fp_CompilerConfigs)
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
                    cout << CreateColouredText("Current BongoJam Compiler Version: ", Colours::BrightMagenta) << CreateColouredText(pm_BongoCompilerVersion, Colours::BrightCyan) << "\n";
                    return BONGO_OK;
                }
            }

            for (int _i = 1; _i < fp_ArgCount; ++_i)
            {
                string f_CompilerArg = fp_ArgVector[_i];

                if (f_CompilerArg == "-cwd")
                {
                    //////////////////// find main function and scripts path - check ////////////////////

                    int result = ScanCwdRecursivelyForScripts();

                    if (result != BONGO_OK)
                    {
                        return result;
                    }

                    //////////////////// unique main script found, and all scripts for project, time to start compilation of each script -> CompilationUnit ////////////////////

                    fp_CompilerConfigs.OutputFileName = "rawr_uwu"; //quick extension substitution (later at compile time)
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
                        fp_CompilerConfigs.BongoScripts.clear(); //just in case some dumbass does "script script proj script"
                        LoadProject(f_CompilerArg, fp_CompilerConfigs); //assuming the proj file is passed as ../../somefolder/name.bsproj i dont think thats a bold assumption

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

                        fp_CompilerConfigs.BongoScripts.push_back(f_Script); //add script to current compiler configs
                    }

                    if (f_CompilerArg != "]")
                    {
                        bongo_logger->Fatal("No ']' found terminating list of scripts after -sp command", "ParseArguments");
                        return NO_SCRIPTS_GIVEN;
                    }

                }//
                else if (f_CompilerArg == "--build_exe")
                {
                    if (fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_DYNAMIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE OPTION SELECTED: tried to pass --build_exe when build_dynamic was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else if (fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_STATIC_LIBRARY)
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
                        fp_CompilerConfigs.CompilerFlags = fp_CompilerConfigs.CompilerFlags | BongoCompilerFlags::BUILD_EXECUTABLE;
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

                    fp_CompilerConfigs.OutputFileName = fp_ArgVector[++_i];  // Increment `_i` to skip the next argument, which is the filename
                }
                else if (f_CompilerArg == "--debug")
                {
                    if (f_IsBuildTypeSpecified)
                    {
                        bongo_logger->Warning("Build type already specified, using first mention and ignoring any repeated mention", "ParseArguments");
                        continue;
                    }

                    fp_CompilerConfigs.CompilerFlags = fp_CompilerConfigs.CompilerFlags | BongoCompilerFlags::DEBUG;
                    f_IsBuildTypeSpecified = true;
                }
                else if (f_CompilerArg == "--release")
                {
                    if (f_IsBuildTypeSpecified)
                    {
                        bongo_logger->Warning("Build type already specified, using first mention and ignoring any repeated mention", "ParseArguments");
                        continue;
                    }

                    fp_CompilerConfigs.CompilerFlags = fp_CompilerConfigs.CompilerFlags | BongoCompilerFlags::RELEASE;
                    f_IsBuildTypeSpecified = true;
                }
                else if (f_CompilerArg == "--compilerun")
                {
                    if (fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_DYNAMIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE BUILD TYPE SELECTED: tried to pass --compilerun when build_dynamic was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else if (fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::BUILD_STATIC_LIBRARY)
                    {
                        bongo_logger->Warning("INCOMPATIBLE BUILD TYPE SELECTED: tried to pass --compilerun when build_static was already selected, cannot create static lib and executable at the same time", "ParseArguments");
                        continue;
                    }
                    else
                    {
                        f_IsCompileRun = true;
                        fp_CompilerConfigs.CompilerFlags = fp_CompilerConfigs.CompilerFlags | BongoCompilerFlags::BUILD_EXECUTABLE;
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

                    fp_CompilerConfigs.CompilerFlags = fp_CompilerConfigs.CompilerFlags | BongoCompilerFlags::BUILD_STATIC_LIBRARY;
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

                    fp_CompilerConfigs.CompilerFlags = fp_CompilerConfigs.CompilerFlags | BongoCompilerFlags::BUILD_DYNAMIC_LIBRARY;

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

            return RunCommands(fp_CompilerConfigs, f_IsCompileRun); //if its not BONGO_OK or whatever then rip uwu cant do anything ab it at this point OwO >O<
        }

        int
            RunCommands(const CompilerConfigs& fp_CompilerConfigs, bool fp_IsCompileRun)
        {
            StartCompilationOfProject(fp_CompilerConfigs);

            if (fp_IsCompileRun)
            {
                if(fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::DEBUG)
                {
                    auto BongoJam_Timer_Start = chrono::high_resolution_clock::now();
                    pm_Interpreter->RunBongoScript(fp_CompilerConfigs.OutputDirectory + "/" + fp_CompilerConfigs.OutputFileName + ".bongo");
                    auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

                    auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);
                }
                else
                {
                    pm_Interpreter->RunBongoScript(fp_CompilerConfigs.OutputDirectory + "/" + fp_CompilerConfigs.OutputFileName + ".bongo");
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
            ScanCwdRecursivelyForScripts(const filesystem::path& fp_RootPath = filesystem::current_path())
        {
            for (const auto& entry : filesystem::recursive_directory_iterator(fp_RootPath))
            {
                string f_FileName = entry.path().filename().string();
                size_t f_Dot = f_FileName.rfind('.');

                if (entry.is_regular_file())
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
            //compile main separately and idk if before or after is smart
            //read file paths into a job queue

            //uint32_t f_CurrentPriorityLevel = 0;
            //used to track when each compiler has been assigned, so that each priority group is guaranteed to be processed first, so if
            //one compiler gets one source file per priority group it ensures that the same compiler isnt being used by multiple threads
            //this is needed since each compiler has its own state due to the presence a logger, flags are the same across projects so state information like that doesn't matter
            //and doing this is worth it if the alternative is losing logging and compilation info since those error strings still gotta be put somewhere idk maybe theres a better way
            //with mt threaded queuing but that has its own downsides idk, logging is fine as is maybe i can even just pass a handle to a logger, however if that logger is used multiple times
            //its string buffer probably wont like that idk each log file is hashed as well so that's not gonna go over well but its static so readonly ops should be fine idk

            pm_CompilerThreadPool.EnqueueTask({ pm_FoundMains[0].FilePath.string() , pm_FoundMains[0].CompiledUnit.get() });

            for (int _i = 0; _i < pm_CurrentProjectSources.size(); ++_i)
            {
                const string f_SourcePath = pm_CurrentProjectSources[_i].FilePath.string();
                CompilationUnit* f_CompilationUnit = pm_CurrentProjectSources[_i].CompiledUnit.get();

                pm_CompilerThreadPool.EnqueueTask({ f_SourcePath , f_CompilationUnit });
            }

            pm_CompilerThreadPool.WaitUntilAllTasksComplete();

            /////// THIS IS ONLY FOR TESTING UWU
            pm_Linker->WriteBytecodeToFile(pm_FoundMains[0].CompiledUnit->CompiledByteCode, fp_CompilerConfigs.OutputDirectory, fp_CompilerConfigs.OutputFileName);
            return BONGO_OK;

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

        //void
        //    CompileRunProject
        //    (
        //        const string& fp_MainFilePath,
        //        const string& fp_OutputFileName,
        //        const string& fp_OutputDirectory
        //    )
        //{
        //    Configs.PrintConfigsToConsole();

        //    StartCompilationOfProject();
        //    

        //    auto BongoJam_Timer_Start = chrono::high_resolution_clock::now();
        //    //pm_Interpreter->RunBongoScript(Configs.m_BongoFileOutputDirectory + "/" + Configs.m_OutputFileName + ".bongo");
        //    auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

        //    auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);

        //    // Output the time taken by bongojam
        //    cout
        //        << CreateColouredText("\nTime taken by bongojam interpreter: ", Colours::BrightYellow)
        //        << BongoJam_Runtime_Duration.count()
        //        << CreateColouredText(" microseconds", Colours::BrightBlue)
        //        << "\n\n\n";
        //}

    };
}//namespace BongoJam