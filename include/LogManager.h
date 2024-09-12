/*******************************************************************
 *                                        BongoJam Script v0.3                                        
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

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <chrono>
#include <iomanip>
#include <sstream>

#include <map>

using namespace std;

namespace BongoJam{

    static string
		CreateColouredText
        (
            const string& fp_SampleText, 
            const string& fp_DesiredColour
        )
	{
		//////////////////// Regular Colours ////////////////////
        
		if (fp_DesiredColour == "black")
		{
			return "\x1B[30m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "red")
		{
			return "\x1B[31m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "green")
		{
			return "\x1B[32m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "yellow")
		{
			return "\x1B[33m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "blue")
		{
			return "\x1B[34m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "magenta")
		{
			return "\x1B[35m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "cyan")
		{
			return "\x1B[36m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "white")
		{
			return "\x1B[37m" + fp_SampleText + "\033[0m";
		}

		//////////////////// Bright Colours ////////////////////

		else if (fp_DesiredColour == "bright black")
		{
			return "\x1B[90m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright red")
		{
			return "\x1B[91m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright green")
		{
			return "\x1B[92m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright yellow")
		{
			return "\x1B[93m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright blue")
		{
			return "\x1B[94m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright magenta")
		{
			return "\x1B[95m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright cyan")
		{
			return "\x1B[96m" + fp_SampleText + "\033[0m";
		}
		else if (fp_DesiredColour == "bright white")
		{
			return "\x1B[97m" + fp_SampleText + "\033[0m";
		}

		//////////////////// Just Return the Input Text Unaltered Otherwise ////////////////////

		else
		{
			return fp_SampleText;
		}
	}

    static void
        Print
        (
            const string& fp_Message, 
            const string& fp_DesiredColour = "white"
        )
    {
        cout << CreateColouredText(fp_Message, fp_DesiredColour) << "\n";
    }

    static void
        PrintError
        (
            const string& fp_Message, 
            const string& fp_DesiredColour = "red"
        )
    {
        cerr << CreateColouredText(fp_Message, fp_DesiredColour) << "\n";
    }

    class LogManager
    {

    //////////////////////////////////////////////
	// Class Destructor
	//////////////////////////////////////////////

    public:
        ~LogManager() 
        {
            FlushAllLogs();  // Ensure all logs are flushed before destruction

            for (auto& _f : pm_LogFiles) 
            {
                if (_f.second.is_open()) 
                {
                    _f.second.close();
                }
            }
        }

        static LogManager& Logger() 
        {
            static LogManager logger;
            return logger;
        }

    //////////////////////////////////////////////
	// Private Class Members
	//////////////////////////////////////////////

    private:

        bool pm_HasBeenInitialized = false;

        map<string, ofstream> pm_LogFiles;

        LogManager() {}

    //////////////////////////////////////////////
	// Public Methods
	//////////////////////////////////////////////

    public:

        void 
            Initialize
            (
                const string& fp_DesiredOutputDirectory,
                const string& fp_MinLogLevel = "trace",
                const string& fp_MaxLogLevel = "fatal"
            ) 
        {
            if (pm_HasBeenInitialized) //stops accidental reinitialization of logmanager
            {
                PrintError("LogManager has already been initialized, LogManager is only allowed to initialize once per run");
                return;
            }

            // Ensure log directory exists
            if (!filesystem::exists(fp_DesiredOutputDirectory)) 
            {
                try 
                {
                    filesystem::create_directories(fp_DesiredOutputDirectory);
                }
                catch (const exception& ex)
                {
                    PrintError("An error occurred inside LogManager: " + static_cast<string>(ex.what()));
                    return;
                }
            }

            vector<string> f_LogLevels = { "trace", "debug", "info", "warn", "error", "fatal", "all-logs" };
            bool f_ShouldInclude = false;

            for (const auto& _level : f_LogLevels) 
            {
                if(_level == fp_MinLogLevel)
                {
                    f_ShouldInclude = true;
                }
                else if(_level == fp_MaxLogLevel and fp_MaxLogLevel != "fatal")
                {
                    f_ShouldInclude = false;
                }

                if
                (
                    f_ShouldInclude or 
                    (_level == "all-logs" and fp_MinLogLevel != fp_MaxLogLevel) //we do this because we don't need all-logs if only one level is desired for some reason lol
                )
                {
                    CreateLogFile(fp_DesiredOutputDirectory, _level + ".log");
                }
            }

            pm_HasBeenInitialized = true; //well if everything went as planned we should be good to set this to true uwu
        }

        void
            Initialize
            (
                const string& fp_DesiredOutputDirectory,
                const vector<string>* fp_DesiredLogLevels
            )
        {
            if (!fp_DesiredLogLevels)
            {
                PrintError("LogManager tried to initialize with a null'd value for specific log filtering");
                return;
            }

            if (pm_HasBeenInitialized) //stops accidental reinitialization of logmanager
            {
                PrintError("LogManager has already been initialized, LogManager is only allowed to initialize once per run");
                return;
            }

            // Ensure log directory exists
            if (!filesystem::exists(fp_DesiredOutputDirectory))
            {
                try
                {
                    filesystem::create_directories(fp_DesiredOutputDirectory);
                }
                catch (const exception& ex)
                {
                    PrintError("An error occurred inside LogManager: " + static_cast<string>(ex.what()));
                    return;
                }
            }

            const vector<string> f_AllowedLogLevels = { "trace", "debug", "info", "warn", "error", "fatal", "all-logs" };

            for (const auto& _level : *fp_DesiredLogLevels)
            {
                CreateLogFile(fp_DesiredOutputDirectory, _level + ".log");

                if (count(f_AllowedLogLevels.begin(), f_AllowedLogLevels.end(), _level) == 0)
                {
                    PrintError("Invalid log level was input when filtering for individual log files");
                    return;
                }
            }
            
            pm_HasBeenInitialized = true; //well if everything went as planned we should be good to set this to true uwu
        }

		//////////////////// Flush All Logs ////////////////////

        void
            FlushAllLogs() 
        {
            for (auto& _f : pm_LogFiles) 
            {
                if (_f.second.is_open()) 
                {
                    _f.second.flush();
                }
            }
        }

        //////////////////// Logging Functions  ////////////////////

        void 
            Log
            (
                const string& fp_Message, 
                const string& fp_Sender,
                const string& fp_LogLevel
            ) 
        {
            string f_TimeStamp = GetCurrentTimestamp();
            string f_LogEntry = "[" + f_TimeStamp + "]" + "[" + fp_LogLevel + "]" + "[" + fp_Sender + "]: " + fp_Message + "\n";

            // Log to specific file and all-logs file
            string f_LogFileName = fp_LogLevel + ".log";

            if (pm_LogFiles.find(f_LogFileName) != pm_LogFiles.end() and pm_LogFiles[f_LogFileName].is_open()) 
            {
                pm_LogFiles[f_LogFileName] << f_LogEntry;
            }
            if (pm_LogFiles.find("all-logs.log") != pm_LogFiles.end() and pm_LogFiles["all-logs.log"].is_open()) 
            {
                pm_LogFiles["all-logs.log"] << f_LogEntry;
            }
        }

        void 
            LogAndPrint
            (
                const string& fp_Message,
                const string& fp_Sender, 
                const string& fp_LogLevel
            ) 
        {
            string f_LogEntry = fp_LogLevel + ": [" + fp_Sender + "] " + fp_Message + "\n";

            // Log to console
            if (fp_LogLevel == "trace")
            {
                Print(fp_Message, "bright white");
            }
            else if (fp_LogLevel == "debug")
            {
                Print(fp_Message, "bright blue");
            }
            else if (fp_LogLevel == "info")
            {
                Print(fp_Message, "bright green");
            }
            else if (fp_LogLevel == "warn")
            {
                Print(fp_Message, "bright yellow");
            }
            else if (fp_LogLevel == "error")
            {
                PrintError(fp_Message, "red"); //not bright oooo soo dark and moody and complex and hard to reach and engage with ><
            }
            else if (fp_LogLevel == "fatal")
            {
                PrintError(fp_Message, "magenta");
            }
            else
            {
                Log("Did not input a valid option for log level in LogAndPrint()", "LogManager", "error");
                Print(fp_Message);
            }

            Log(fp_Message, fp_Sender, fp_LogLevel);
        }

    //////////////////////////////////////////////
    // Private Methods
    //////////////////////////////////////////////

    private:

        void 
            CreateLogFile
            (
                const string& fp_FilePath,
                const string& fp_FileName
            ) 
        {
            ofstream f_File;

            f_File.open(fp_FilePath + "/" + fp_FileName, ios::out | ios::app);

            if (!f_File.is_open()) 
            {
                cerr << "Failed to open log file: " << fp_FileName << "\n";
            } 
            else 
            {
                pm_LogFiles[fp_FileName] = move(f_File);
            }
        }

        string //thank you chat-gpt uwu
            GetCurrentTimestamp() 
        {
            auto now = chrono::system_clock::now();
            auto time_t_now = chrono::system_clock::to_time_t(now);
            auto local_time = *localtime(&time_t_now);

            stringstream ss;
            ss << put_time(&local_time, "%Y-%m-%d %H:%M:%S");

            auto since_epoch = now.time_since_epoch();
            auto milliseconds = chrono::duration_cast<chrono::milliseconds>(since_epoch).count() % 1000;

            ss << '.' << setfill('0') << setw(3) << milliseconds;

            return ss.str();
        }
    };
}