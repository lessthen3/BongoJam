#pragma once

#include <string>
#include <iostream>

#include "LogManager.h"

using namespace std;

namespace BongoJam {

	static string
		CreateColouredText(string fp_SampleText, string fp_DesiredColour)
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
		Log(string fp_Message, string fp_Sender, string fp_LogLevel)
	{
		if (fp_LogLevel == "trace")
		{
			LogManager::MainLogger().Trace(fp_Message, fp_Sender);
		}
		else if (fp_LogLevel == "debug")
		{
			LogManager::MainLogger().Debug(fp_Message, fp_Sender);
		}
		else if (fp_LogLevel == "info")
		{
			LogManager::MainLogger().Info(fp_Message, fp_Sender);
		}
		else if (fp_LogLevel == "warn")
		{
			LogManager::MainLogger().Warn(fp_Message, fp_Sender);
		}
		else if (fp_LogLevel == "error")
		{
			LogManager::MainLogger().Error(fp_Message, fp_Sender);
		}
		else if (fp_LogLevel == "fatal")
		{
			LogManager::MainLogger().Fatal(fp_Message, fp_Sender);
		}
		else
		{
			LogManager::MainLogger().Error("Did not input a valid option for log level in LogAndPrint()", "Tools");
			cout << CreateColouredText(fp_Message, "white") << "\n";
		}
	}

	static void
		LogAndPrint(string fp_Message, string fp_Sender, string fp_LogLevel, string fp_TextColour = "white")
	{
		if (fp_LogLevel == "trace")
		{
			LogManager::MainLogger().Trace(fp_Message, fp_Sender);
			cout << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
		else if (fp_LogLevel == "debug")
		{
			LogManager::MainLogger().Debug(fp_Message, fp_Sender);
			cout << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
		else if (fp_LogLevel == "info")
		{
			LogManager::MainLogger().Info(fp_Message, fp_Sender);
			cout << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
		else if (fp_LogLevel == "warn")
		{
			LogManager::MainLogger().Warn(fp_Message, fp_Sender);
			cout << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
		else if (fp_LogLevel == "error")
		{
			LogManager::MainLogger().Error(fp_Message, fp_Sender);
			cerr << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
		else if (fp_LogLevel == "fatal")
		{
			LogManager::MainLogger().Fatal(fp_Message, fp_Sender);
			cerr << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
		else
		{
			Log("Did not input a valid option for log level in LogAndPrint()", "Tools", "error");
			cout << CreateColouredText(fp_Message, fp_TextColour) << "\n";
		}
	}

}