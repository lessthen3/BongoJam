#include "../include/LogManager.h"
#include <fstream>
#include <iostream>

using namespace std;

namespace BongoJam {

    void 
        LogManager::Initialize(const string& logDirectory, const string& fp_LoggerName = "MainLogger") 
    {

        if (hasBeenInitialized) //stops accidental reinitialization of logmanager
        {
            cerr << "LogManager has already been initialized, LogManager is only allowed to initialize once per run\n";
            return;
        }
        else if (fp_LoggerName == "") //prevents any directory nonsense
        {
            cerr << "Stop playin and input a logger name that isnt an empty string, because it's going to break the directory structure otherwise" << "\n";
            return;
        }
        m_LoggerName = fp_LoggerName;

        // Ensure log directory exists
        if (!filesystem::exists(logDirectory)) 
        {
            try 
            {
                filesystem::create_directories(logDirectory);
            }
            catch (const exception& ex)
            {
                cerr << "An error occurred: " << ex.what() << endl;
                return;
            }
            
        }
        // Create log files if they don't exist
        CreateLogFiles(logDirectory);
        
        // Define sinks
        vector<spdlog::sink_ptr> sinks;

        // Add console sink
        sinks.push_back(make_shared<spdlog::sinks::stdout_color_sink_mt>());

        // Add file sinks for every debug level plus an extra sink that contains all log levels in one file
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/trace.log", true));
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/debug.log", true));
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/info.log", true));
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/warn.log", true));
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/error.log", true));
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/fatal.log", true));
        sinks.push_back(make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory + "/" + fp_LoggerName + "/all-logs.log", true));

        // Create and register the logger
        logger = make_shared<spdlog::logger>(fp_LoggerName, sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);  // Set the logger's level to trace
        logger->flush_on(spdlog::level::trace);   // Flush the logger on every trace log message

        spdlog::register_logger(logger);  // Register the logger with spdlog
       
        hasBeenInitialized = true; //logger should be fully initialized successfully
    }

    void LogManager::Trace(const string& message, const string& className) {
        logger->trace("[{}] {}", className, message);
    }

    void LogManager::Debug(const string& message, const string& className) {
        logger->debug("[{}] {}", className, message);
    }

    void LogManager::Info(const string& message, const string& className) {
        logger->info("[{}] {}", className, message);
    }

    void LogManager::Warn(const string& message, const string& className) {
        logger->warn("[{}] {}", className, message);
    }

    void LogManager::Error(const string& message, const string& className) {
        logger->error("[{}] {}", className, message);
    }

    void LogManager::Fatal(const string& message, const string& className) {
        logger->critical("[{}] {}", className, message);
    }

    void 
        LogManager::CreateLogFiles(const string& logDirectory) 
    {
        vector<string> logFiles = {
            "trace.log",
            "debug.log",
            "info.log",
            "warn.log",
            "error.log",
            "fatal.log",
            "all-logs.log"
        };

        for (const auto& file : logFiles)
        {
            string filePath = logDirectory + "/" + m_LoggerName + "/" + file;
            if (!filesystem::exists(filePath)) 
            {
                ofstream ofs(filePath); // This will create the file if it doesn't exist
            }
        }
    }
}