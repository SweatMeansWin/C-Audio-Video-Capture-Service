#pragma once

#include "SystemLog.h"
#include "CommonSettings.h"

struct CEventLogger
{
	void SetSettings(const TSaveSettings& NewSettings);

private:
	bool				m_bLocalChecking;
	bool				m_errorCopying;
	TSaveSettings		m_settings;

	void impersonateConnection(const std::wstring& sPath);

	void writeLog(const TActualLogEvent& info, const std::wstring& sFilename, bool moved);
	void moveRecord(const TActualLogEvent& info);
	bool moveLocalRecord(const TActualLogEvent& info);
};
