//
// Created by evgen on 09.05.2018.
//

#include "PreCompiled.h"
#include "Logging.h"
#include <chrono>
#include <iostream>

ELogLevel Log::reportingLevel = ELogLevel::LOGINFO;

std::ostream &operator<<(std::ostream &os, ELogLevel logLevel) {
	switch (logLevel) {
	case ELogLevel::LOGERROR:
		return os << "ERROR";
	case ELogLevel::LOGWARNING:
		return os << "WARN";
	case ELogLevel::LOGINFO:
		return os << "INFO";
	case ELogLevel::LOGDEBUG:
		return os << "DEBUG";
	default:
		return os << "UNKNOWN";
	};
	return os;
}

std::ostream &operator<<(std::ostream &os, std::wstring ws) {
	return os << std::string(ws.cbegin(), ws.cend());
}

std::string getCurrentTime() {
	time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char str[32];
	tm time = tm();
	localtime_s(&time, &t);
	strftime(str, 32, "%y:%m:%d %X", &time);
	return std::string(str);
}

std::ostringstream &Log::Get(ELogLevel level) {
	outputStream << "-" << getCurrentTime() << " (" << level << "): ";
	m_logLevel = level;
	return outputStream;
}

void Log::SetReportingLevel(ELogLevel logLevel) {
	reportingLevel = logLevel;
}

ELogLevel Log::GetReportingLevel() {
	return reportingLevel;
}

Log::Log() : m_logLevel(ELogLevel::LOGDEBUG) {}

Log::~Log() {
	if (m_logLevel <= reportingLevel) {
		std::cout << outputStream.str() << std::endl;
	}
}
