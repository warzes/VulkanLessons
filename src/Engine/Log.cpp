#include "stdafx.h"
#include "Log.h"
//-----------------------------------------------------------------------------
void LogPrint(const std::string& text)
{
	puts(text.c_str());
}
//-----------------------------------------------------------------------------
void LogWarning(const std::string& text)
{
	LogPrint("WARNING: " + text);
}
//-----------------------------------------------------------------------------
void LogError(const std::string& text)
{
	LogPrint("ERROR: " + text);
}
//-----------------------------------------------------------------------------
void Fatal(const std::string& text)
{
	LogPrint("FATAL: " + text);
	extern bool IsExit;
	IsExit = true;
}
//-----------------------------------------------------------------------------