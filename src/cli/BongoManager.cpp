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
#include "BongoManager.h"

//static helpers 
namespace BongoJam {
    [[nodiscard]] static char**
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

    static void
        FreeCharArray(char** charArray)
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

    static void
        DisplayHelp()
    {
        cout
            << CreateColouredText("{Usage}: bongo <file> [options]...\n", Colours::BrightMagenta)

            << CreateColouredText("Compiler Options:\n  ", Colours::BrightYellow) //oh it's because i put two spaces after each \n lmao
            << CreateColouredText("  -o <filename>\tSet output filename\n  ", Colours::Cyan) //this needs an extra two spaces at the beginning for god knows what reason ?_? !!
            << CreateColouredText("  -d <directory>\tSet output directory\n  ", Colours::Cyan)

            << CreateColouredText("  --compile-run\tIf used, the script will be compiled and ran immediately\n  ", Colours::Cyan)
            << CreateColouredText("  --debug\t\tCompile in debug mode\n  ", Colours::Cyan)
            << CreateColouredText("  --pedantic\t\tCompile with all warnings turned on\n  ", Colours::Cyan)

            << CreateColouredText("  --clear-logs\tClears Desired Log Files\n", Colours::Cyan)
            << CreateColouredText("\t  [option 1] LOG_LEVEL_MINOR - LOG_LEVEL_MAJOR\n", Colours::BrightGreen)
            << CreateColouredText("\t  [option 2] LOG_LEVEL_1, LOG_LEVEL_2 . . .\n  ", Colours::BrightGreen)

            << CreateColouredText("  --set LOG_LEVEL_FILTER\t Filters Log Output\n  ", Colours::Cyan) //disable/enable internal logs, and set the min and max log level, one arg is min, two args is both
            << CreateColouredText("\t  [option 1] LOG_LEVEL_MINOR - LOG_LEVEL_MAJOR\n", Colours::BrightGreen)
            << CreateColouredText("\t  [option 2] LOG_LEVEL_1, LOG_LEVEL_2 . . .\n  ", Colours::BrightGreen)

            << CreateColouredText("  --set LOG_OUTPUT_DIRECTORY <directory>\tSets Working Log Output Directory\n  ", Colours::Cyan)
            << CreateColouredText("  --set DEFAULT_OUTPUT_DIRECTORY <directory>\tSets Default Log Output Directory\n  ", Colours::Cyan)

            << CreateColouredText("  -h, --help\tDisplay this help and exit\n  ", Colours::Cyan)
            << CreateColouredText("  --version\tGet the currently installed compiler version\n  ", Colours::Cyan)
            ;
    }
    
    [[nodiscard]] static int
        ScanDirectoryRecursivelyForScripts
        (
            const filesystem::path& fp_RootPath,
            BongoProject& fp_BongoProject,
            Logger* logger
        )
    {
        if (not fp_BongoProject.MainPath.empty())
        {
            logger->Warning("INTERNAL: tried passing a non-emptry path var for main source path", "CLI");
            fp_BongoProject.MainPath.clear();
        }

        if (not fp_BongoProject.SourcePaths.empty())
        {
            logger->Warning("INTERNAL: tried passing a non-emptry path list var for script source paths", "CLI");
            fp_BongoProject.SourcePaths.clear();
        }

        for (const auto& lv_DirectoryEntry : filesystem::recursive_directory_iterator(fp_RootPath))
        {
            string f_FileName = lv_DirectoryEntry.path().filename().string();
            size_t f_Dot = f_FileName.rfind('.');

            if (lv_DirectoryEntry.is_regular_file() and f_Dot != string::npos)
            {
                if (f_FileName == "main.bj")
                {
                    if (not fp_BongoProject.MainPath.empty())
                    {
                        logger->Fatal("Multiple main.bj scripts found! Please make sure only one exists.", "CLI");
                        logger->Error(fmt::format("found: {}  ---  {}", fp_BongoProject.MainPath.root_path().string(), lv_DirectoryEntry.path().root_path().string()), "CLI");
                        return BONGO_MULTIPLE_MAINS_FOUND;
                    }

                    logger->Debug(fmt::format("main.bj found at : '{}'", lv_DirectoryEntry.path().string()), "CLI");
                    fp_BongoProject.MainPath = lv_DirectoryEntry.path();
                }
                else if (f_FileName.substr(f_Dot) == ".bj")
                {
                    fp_BongoProject.SourcePaths.emplace_back(lv_DirectoryEntry.path());
                }
                else if (f_FileName.substr(f_Dot) == ".bsproj") //only assumes one .bsproj in the directory atm will just use the last one found as the bsproj currently
                {
                    fp_BongoProject.ProjectFilePath = lv_DirectoryEntry.path();
                }
            }
        }

        if (fp_BongoProject.MainPath.empty())
        {
            logger->Fatal("No main.bj script found anywhere in project directory!", "CLI");
            return BONGO_NO_MAIN_FOUND;
        }

        return BONGO_OK;
    }
}

namespace BongoJam
{
    //////////////////////////////////////////////
    // Constructor 
    //////////////////////////////////////////////

    CLI::CLI()
    {
        bongo_logger = Logger::CreateUnique("CLI", PEACH_LOGGER_DEFAULT_FLAGS, PEACH_LOGGER_DEFAULT_OUTPUT_DIR);

        if (not bongo_logger)
        {
            throw runtime_error("WTF MANG LOGGER FAILED TO INITIALIZE FROM THREADPOOL WTF MANG");
        }

        bongo_logger->Debug("uwu", "CLI");

        pm_Linker = make_unique<BongoLinker>();
        pm_Interpreter = make_unique<BongoJamInterpreter>();
    }

    int
        CLI::LoadProject(const string& fp_ProjectDirectory, CompilerConfigs& fp_CompilerConfigs)
    {
        if (not PEACH_FROM_JSON(pm_CurrentProject.Project, fp_ProjectDirectory, bongo_logger.get()))
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
        CLI::AddScriptsToExistingProject()
    {

        return BONGO_OK;
    }

    static string
        ParseWord()
    {

        return "";
    }

    int
        CLI::ParseArguments(const vector<string>& fp_Args) //for easier use from C++
    {
        char** f_CharArray = StringVectorToCharArray(fp_Args);

        int result = ParseArguments(fp_Args.size(), f_CharArray);

        FreeCharArray(f_CharArray);

        return result;
    }

    int
        CLI::ParseArguments(int fp_ArgCount, char* fp_ArgVector[])
    {
        if (fp_ArgCount < 2)
        {
            PRINT("No arguments provided. Use -h or --help for usage information.", Magenta);
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

        bool f_UsingSpecifiedScripts = false;

        for (int _i = 1; _i < fp_ArgCount; ++_i) //start at one, bongo is the first arg uwu
        {
            string f_CompilerArg = fp_ArgVector[_i];

            //if (_i == 1) // first arg is always named scripts, if they just use -cwd or smth it'll be caught by falling through
            //{
            //    _i++; //advance forward onemore step

            //    if (not (_i < fp_ArgCount)) //check for bounds
            //    {
            //        bongo_logger->Fatal("No files input for compiler, try -cwd if you're looking for automatic script searching in the current working directory ~w~ ~~nyah", "ParseArguments");
            //        return NO_SCRIPTS_GIVEN;
            //    }

            //    f_CompilerArg = string(fp_ArgVector[_i]);
            //    size_t f_Dot = f_CompilerArg.rfind('.');

            //    if (f_Dot != string::npos and f_CompilerArg.substr(f_Dot) == ".bsproj")
            //    {
            //        pm_CurrentProject.Project.LastUsedCompilerConfigs.BongoScripts.clear(); //just in case some dumbass does "script script proj script"
            //        LoadProject(f_CompilerArg, pm_CurrentProject.Project.LastUsedCompilerConfigs); //assuming the proj file is passed as ../../somefolder/name.bsproj i dont think thats a bold assumption

            //        continue;
            //    }
            //    
            //    while (_i + 1 < fp_ArgCount) //parse forever uwu
            //    {
            //        ++_i;

            //        f_CompilerArg = string(fp_ArgVector[_i]);
            //        BongoScriptFile f_Script;

            //        f_Script.ScriptName = f_CompilerArg;
            //        size_t f_Dot = f_Script.ScriptName.rfind('.');

            //        f_Script.ScriptFilePath = "./" + f_Script.ScriptName; // we add before hand because we want a bj here :^)

            //        if (not filesystem::exists(f_Script.ScriptName))
            //        {
            //            PrintError("No script with name: " + f_Script.ScriptName + " found at specified file path OwO", Colours::Magenta);
            //            return SCRIPT_DOES_NOT_EXIST;
            //        }

            //        if (f_Dot != string::npos and f_Script.ScriptName.substr(f_Dot) == ".bj")
            //        {
            //            f_Script.ScriptName = f_Script.ScriptName.substr(0, f_Dot);
            //        }
            //        else
            //        {
            //            PrintError("Invalid script target found, please only try to compile .bj files only", Colours::Magenta);
            //            return INVALID_SCRIPT_TARGET;
            //        }

            //        pm_CurrentProject.Project.LastUsedCompilerConfigs.BongoScripts.push_back(f_Script); //add script to current compiler configs
            //    }

            //    f_UsingSpecifiedScripts = true;
            //}//

            if (f_CompilerArg == "-cwd")
            {
                if (f_UsingSpecifiedScripts)
                {
                    bongo_logger->Fatal("No files input for compiler after '-sp' flag was used, try -cwd if you're looking for automatic script searching", "ParseArguments");
                    return INVALID_SCRIPT_TARGET;
                }
                //////////////////// find main function and scripts path - check //////////////////// TODO: THIS IS STUPID

                BongoProject f_TempProject;

                int result = ScanDirectoryRecursivelyForScripts(filesystem::current_path(), f_TempProject, bongo_logger.get());

                if (result != BONGO_OK)
                {
                    return result;
                }

                //////////////////// unique main script found, and all scripts for project, time to start compilation of each script -> CompilationUnit ////////////////////

                pm_CurrentProject.Project.LastUsedCompilerConfigs.OutputFileName = "rawr_uwu"; //quick extension substitution (later at compile time)
            }
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
            else if (f_CompilerArg == "-o")
            {
                if (not (_i + 1 < fp_ArgCount)) //check for bounds
                {
                    bongo_logger->Fatal("No output file name was input after '-o' flag was used, please input a valid file name after [-o] is used", "ParseArguments");
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
                PRINT("Unknown or incomplete argument provided: " + f_CompilerArg, Magenta);
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

        return BONGO_OK;//RunCommands(pm_CurrentProject.Project.LastUsedCompilerConfigs, f_IsCompileRun); //if its not BONGO_OK or whatever then rip uwu cant do anything ab it at this point OwO >O<
    }


    //int
    //    CLI::RunCommands(const CompilerConfigs& fp_CompilerConfigs, bool fp_IsCompileRun)
    //{
    //    if (StartCompilationOfProject(fp_CompilerConfigs) != BONGO_OK)
    //    {
    //        bongo_logger->Error("Compilation Failed! nothing was done.", "RunCommands");
    //        return BONGO_COMPILATION_FAILED;
    //    }

    //    if (fp_IsCompileRun)
    //    {
    //        if(fp_CompilerConfigs.CompilerFlags & BongoCompilerFlags::DEBUG)
    //        {
    //            const auto BongoJam_Timer_Start = chrono::high_resolution_clock::now();
    //            pm_Interpreter->BongoTime(fp_CompilerConfigs.OutputDirectory + "/" + fp_CompilerConfigs.OutputFileName + ".bongo");
    //            const auto BongoJam_Timer_Stop = chrono::high_resolution_clock::now();

    //            const auto BongoJam_Runtime_Duration = chrono::duration_cast<chrono::microseconds>(BongoJam_Timer_Stop - BongoJam_Timer_Start);
    //        }
    //        else
    //        {
    //            pm_Interpreter->BongoTime(fp_CompilerConfigs.OutputDirectory + "/" + fp_CompilerConfigs.OutputFileName + ".bongo");
    //        }
    //    }

    //    return EXIT_SUCCESS;
    //}

    int
        CLI::StartCompilationOfProject
        (
            const BongoProject& fp_BongoProject,
            ICompilerThreadPool& fp_CompilerThreadPool
        )
    {
        const size_t f_TaskCount = 1 + fp_BongoProject.SourcePaths.size();  // figure out how many compilation jobs we have, +1 since mains aren't inside the project sources uwu

        if (f_TaskCount < 1)
        {

            return BONGO_NO_SOURCE_FILES_IN_PROJECT_AT_COMPILE_REQUEST; //not the real error but kinda there owo
        }

        //////////////////// Allocate compilation units for each source script + main ////////////////////

        vector<unique_ptr<SSA::CompilationUnit>> f_CompiledScripts;

        f_CompiledScripts.reserve(f_TaskCount);

        for (size_t lv_Index = 0; lv_Index < f_TaskCount; ++lv_Index)
        {
            f_CompiledScripts.emplace_back(make_unique<SSA::CompilationUnit>());
        }

        //////////////////// Start Compilation Pool ////////////////////

        fp_CompilerThreadPool.StartBatch(f_TaskCount);

        // if latch creation failed or something went wrong
        if (not fp_CompilerThreadPool.IsBatchActive())
        {
            bongo_logger->Error("Failed to start compilation batch", "CLI");
            return UNABLE_TO_START_BATCH_COMPILATION; // define this if you haven’t
        }

        if (fp_BongoProject.MainPath.empty())
        {

            return BONGO_NO_MAIN_FOUND_IN_PROJECT_AT_COMPILE_REQUEST;
        }

        //read file paths into a job queue
        fp_CompilerThreadPool.EnqueueTask({ fp_BongoProject.MainPath, f_CompiledScripts[0].get() });

        for (int _i = 0; _i < fp_BongoProject.SourcePaths.size(); ++_i) //start at
        {
            fp_CompilerThreadPool.EnqueueTask({ fp_BongoProject.SourcePaths[_i] , f_CompiledScripts[_i + 1].get()}); //entry point is entry 0
        }

        fp_CompilerThreadPool.WaitUntilAllTasksComplete();

        if (not fp_CompilerThreadPool.BONGO_COMPILE_SUCCESS)
        {

            return BONGO_COMPILATION_FAILED;
        }

        //////////////////// every script was validated and compiled into a CompilationUnit, Linker time baby ////////////////////
        // also need to find precompiled CompilationUnits via configs for external deps
        //link together compilationunits, assuming everything was checked properly, every script unit should have a corresponding compilationunit attached to it uwu
        //run linker to resolve symbols, and return error if found fingys cwossed >w<  

        vector<uint8_t> f_FullBongoProgram;
        pm_Linker->LinkCompilationUnits(std::move(f_CompiledScripts), f_FullBongoProgram);

        //////////////////// Write fully assembled BongoJam program that is ready to be run >O< ////////////////////
        if (
            not FileIO::WriteToBinary
            (
                fp_BongoProject.LastUsedCompilerConfigs.OutputDirectory, 
                fp_BongoProject.LastUsedCompilerConfigs.OutputFileName + ".bongo",
                f_FullBongoProgram, bongo_logger.get())
            )
        {

            return EXIT_FAILURE;
        }

        //after successfully writing bytecode to a file return BONGO_OK
        return BONGO_OK;
    }

#ifdef BONGO_DEBUG
    int
        CLI::RunTest
        (
            const filesystem::path& fp_ScriptPath,
            ICompilerThreadPool& fp_CompilerThreadPool
        )
    {
        //read file paths into a job queue

        fp_CompilerThreadPool.StartBatch(1);

        if (not fp_CompilerThreadPool.IsBatchActive())
        {

            return UNABLE_TO_START_BATCH_COMPILATION;
        }

        unique_ptr<SSA::CompilationUnit> f_CompiledUnit = make_unique<SSA::CompilationUnit>();

        f_CompiledUnit->TU->ScriptPath = fp_ScriptPath;

        fp_CompilerThreadPool.EnqueueTask({ fp_ScriptPath, f_CompiledUnit.get() });

        fp_CompilerThreadPool.WaitUntilAllTasksComplete();

        if (not fp_CompilerThreadPool.BONGO_COMPILE_SUCCESS)
        {

            return BONGO_COMPILATION_FAILED;
        }

        vector<unique_ptr<SSA::CompilationUnit>> f_CompiledProject;
        f_CompiledProject.push_back(std::move(f_CompiledUnit)); //put entry point as last item

        //////////////////// every script was validated and compiled into a CompilationUnit, Linker time baby ////////////////////
        // also need to find precompiled CompilationUnits via configs for external deps
        //link together compilationunits, assuming everything was checked properly, every script unit should have a corresponding compilationunit attached to it uwu
        //run linker to resolve symbols, and return error if found fingys cwossed >w<  

        vector<uint8_t> f_FullBongoProgram;
        pm_Linker->LinkCompilationUnits(std::move(f_CompiledProject), f_FullBongoProgram);

        //////////////////// Write fully assembled BongoJam program that is ready to be run >O< ////////////////////
        if (not FileIO::WriteToBinary("./", "rawr_uwu.bongo", f_FullBongoProgram, bongo_logger.get()))
        {

            return EXIT_FAILURE;
        }

        //after successfully writing bytecode to a file return BONGO_OK
        return pm_Interpreter->BongoTime("./rawr_uwu.bongo");
    }
#endif /*BONGO_DEBUG*/
}//namespace BongoJam