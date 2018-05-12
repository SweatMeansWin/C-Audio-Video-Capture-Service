#pragma once

// SweatMeansWin
// Common interface for logging all level activities

#include <sstream>
#include "Errors.h"

#define LogMessageStream Log().Get(ELogLevel::LOGINFO)
#define LogDebugStream Log().Get(ELogLevel::LOGDEBUG)
#define LogWarningStream Log().Get(ELogLevel::LOGWARNING)
#define LogErrorStream Log().Get(ELogLevel::LOGERROR)

#define LogMessage(LogText) { LogMessageStream << (LogText);}
#define LogDebug(DebugInfo) { LogDebugStream << (DebugInfo);}
#define LogWarning(WarnText) { LogWarningStream << (WarnText);}
#define LogError(ErrorText, ErrorCode) { LogErrorStream << (ErrorText) << ". Error[" << (ErrorCode) << "]";}


enum class ELogLevel : uint16_t {
	LOGERROR,
	LOGWARNING,
	LOGINFO,
	LOGDEBUG
};
std::ostream &operator<<(std::ostream &os, ELogLevel logLevel);
std::ostream &operator<<(std::ostream &os, std::wstring ws);

class Log {
public:
	Log();
	~Log();

	std::ostringstream &Get(ELogLevel level = ELogLevel::LOGINFO);
	
	static void SetReportingLevel(ELogLevel logLevel);
	static ELogLevel GetReportingLevel();

protected:
	std::ostringstream outputStream;

private:
	ELogLevel m_logLevel;
	static ELogLevel reportingLevel;
};
