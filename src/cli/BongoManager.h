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

namespace BongoJam
{
    struct BongoConfigs
    {
        BongoConfigs() = default;
        ~BongoConfigs() = default;

        string pm_LogOutputDirectory = "./logs";
        uint32_t pm_LogFlags = PEACH_LOGGER_DEFAULT_FLAGS;
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
                << CreateColouredText(pm_LogOutputDirectory, Colours::BrightCyan)
                << "\n\n"
                ;
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

        PEACH_SERIALIZABLE
        (
            PEACH_FIELD(ScriptName),
            PEACH_FIELD(ScriptFilePath)
        )
    };

    struct CompilerConfigs //keeps track of compiler settings and all source paths
    {
        string MainFilePath;
        // Name : Path
        vector<BongoScriptFile> BongoScripts;

        string OutputFileName = "rawr_uwu"; //everything gets stiched together into a single IR executable or .bcu (bongo compiled unit aka a static lib + name table for includes and optional commenting)
        string OutputDirectory = "./";

        uint64_t CompilerFlags = BongoCompilerFlags::DEFAULT;

        PEACH_SERIALIZABLE
        (
            PEACH_FIELD(MainFilePath),
            PEACH_FIELD(BongoScripts),
            PEACH_FIELD(OutputFileName),
            PEACH_FIELD(OutputDirectory),
            PEACH_FIELD(CompilerFlags)
        )
    };

    struct BongoProject //serialized to .bsproj file when building
    {
        string ProjectName = "OwO";
        string ProjectVersion = "0.0.1";

        string BongoJamVersion = "0.0.1";
        string BongoCompilerVersion = "0.0.1";

        CompilerConfigs LastUsedCompilerConfigs;

        vector<filesystem::path> SourcePaths;
        filesystem::path MainPath;

        filesystem::path ProjectFilePath;

        PEACH_SERIALIZABLE
        (
            PEACH_FIELD(ProjectName),
            PEACH_FIELD(ProjectVersion),
            PEACH_FIELD(BongoJamVersion),
            PEACH_FIELD(BongoCompilerVersion),
            PEACH_FIELD(LastUsedCompilerConfigs)
        )
    };

    struct CurrentBongoProject //used by bongomanager during runtime
    {
        BongoProject Project;
        filesystem::path pm_CurrentBongoProjectFilePath;
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace BongoJam {

    class CLI
    {
        //////////////////////////////////////////////
        // Constructor and Destructor
        //////////////////////////////////////////////
    public:
        CLI();

        ~CLI() = default;

        CLI(const CLI&) = delete;
        CLI& operator=(const CLI&) = delete;
        CLI(CLI&&) = delete;
        CLI& operator=(CLI&&) = delete;

    public:
        BongoConfigs Configs; //persistent options, bongomanager is used for per session or project basis

    private:
        unique_ptr<Logger> bongo_logger = nullptr;

        unique_ptr<BongoLinker> pm_Linker = nullptr;
        unique_ptr<BongoJamInterpreter> pm_Interpreter = nullptr;

        CurrentBongoProject pm_CurrentProject;

    public:

        [[nodiscard]] int
            LoadProject(const string& fp_ProjectDirectory, CompilerConfigs& fp_CompilerConfigs);

        [[nodiscard]] int
            CreateNewProject();

        [[nodiscard]] int
            AddScriptsToExistingProject();

        [[nodiscard]] int
            ParseArguments(const vector<string>& fp_Args);

        [[nodiscard]] int
            ParseArguments(int fp_ArgCount, char* fp_ArgVector[]);

        //[[nodiscard]] int
        //    RunCommands(const CompilerConfigs& fp_CompilerConfigs, bool fp_IsCompileRun);

        [[nodiscard]] int
            StartCompilationOfProject
            (
                const BongoProject& fp_BongoProject,
                ICompilerThreadPool& fp_CompilerThreadPool
            );

#ifdef BONGO_DEBUG
        int
            RunTest
            (
                const filesystem::path& fp_ScriptPath,
                ICompilerThreadPool& fp_CompilerThreadPool
            );
#endif /*BONGO_DEBUG*/
    };
}//namespace BongoJam