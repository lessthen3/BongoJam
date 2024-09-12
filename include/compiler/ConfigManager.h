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
#pragma once

#include "../LogManager.h"

using namespace std;

namespace BongoJam {

    struct ConfigManager
    {
        const string m_BongoJamVersion = "0.0.6";

        string m_OutputFileName = "rawr"; //default value

        bool m_IsCompileRun = false;
        bool m_IsDebugMode = false;

        string m_LogOutputDirectory = "./logs";
        string m_BongoFileOutputDirectory = "./";

        string m_ScriptName; //will always be initialized outside of the struct since the program will not run otherwise uwu
        string m_ScriptFilePath;

        string m_LogLevelMinor = "trace";
        string m_LogLevelMajor = "fatal";

        vector<string>* m_DesiredLogs = nullptr; //ptr so i can do a bool check

        void
            CreateDefaultConfigs()
            const
        {
            ofstream f_Configs("configs.ini");

            if (!f_Configs)
            {
                PrintError("Failed to create default settings file.");
                return;
            }

            string f_IsDebug = (m_IsDebugMode ? "true" : "false");

            f_Configs << "[Settings]\n";
            f_Configs << "OutputFileName=" + m_OutputFileName + "\n";
            f_Configs << "OutputDirectory=" + m_BongoFileOutputDirectory + "\n";
            f_Configs << "DebugMode= " + f_IsDebug + "\n";
            f_Configs << "LogOutputDirectory= " + m_LogOutputDirectory + "\n";

            f_Configs.close();
        }

        void
            WriteToConfigs()
            const
        {
            ofstream f_Configs("configs.ini");

            if (!f_Configs)
            {
                PrintError("Failed to open config file for writing.");
                return;
            }

            string f_IsDebug = (m_IsDebugMode ? "true" : "false");

            f_Configs << "[Settings]\n";
            f_Configs << "OutputFileName=" + m_OutputFileName + "\n";
            f_Configs << "OutputDirectory=" + m_BongoFileOutputDirectory + "\n";
            f_Configs << "DebugMode= " + f_IsDebug + "\n";
            f_Configs << "LogOutputDirectory= " + m_LogOutputDirectory + "\n";

            f_Configs.close();
        }

        map<string, string>
            ReadConfigs()
            const
        {
            map<string, string> f_Configs;
            ifstream f_ConfigFile("configs.ini");

            if (!f_ConfigFile)
            {
                PrintError("Failed to open config file for reading.", "magenta");
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

        void
            LoadConfigs()
        {
            map<string, string> f_LoadedConfigs = ReadConfigs();

            for (auto& _config : f_LoadedConfigs)
            {
                if (_config.first == "OutputFileName")
                {
                    m_OutputFileName = _config.second;
                }
                else if (_config.first == "OutputDirectory")
                {
                    m_BongoFileOutputDirectory = _config.second;
                }
                else if (_config.first == "DebugMode")
                {
                    m_IsDebugMode = (_config.second == "true"); //anything is not debug mode xdxdxdxdxd
                }
                else if (_config.first == "LogOutputDirectory")
                {
                    m_LogOutputDirectory = _config.second;
                }

            }
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

        ConfigManager() = default;
        ~ConfigManager() = default;
    };
}