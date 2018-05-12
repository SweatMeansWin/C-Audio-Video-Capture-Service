#include "PreCompiled.h"
#include "EventLogger.h"

#include "Errors.h"
#include "Logging.h"
#include "WinHeaders.h"

bool DirectoryExists(const std::wstring& sPath)
{
	DWORD dwAttrib = GetFileAttributes(sPath.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

bool IsEnoughSpace(const std::wstring& path, uint64_t minValue)
{
	ULARGE_INTEGER uli = { 0 };
	if (GetDiskFreeSpaceEx(path.c_str(), nullptr, nullptr, &uli)) 	{
		return uli.QuadPart > minValue;
	}
	return true;
}

void CEventLogger::SetSettings(const TSaveSettings& NewSettings)
{
	if (!DirectoryExists(m_settings.RemoteSavePath)) {
		// create(m_settings.RemoteSavePath);
	}
	if (!DirectoryExists(m_settings.LocalSavePath)) {
		// create(m_settings.RemoteSavePath);
	}
}

void CEventLogger::impersonateConnection(const std::wstring& sPath)
{
	if (m_settings.RemoteLogin.empty()) {
		return;
	}

	NETRESOURCE ns = { 0 };
	ns.lpRemoteName = const_cast<LPWSTR>(sPath.c_str());

	if (!WNetAddConnection2(
		&ns,
		m_settings.RemoteLogin.c_str(),
		m_settings.RemotePassword.c_str(),
		CONNECT_UPDATE_PROFILE
	)) {
		throw BaseError("Unable to connect to remote", ::GetLastError());
	}
}

void CEventLogger::moveRecord(const TActualLogEvent& info)
{
	std::wstring sFilenameInShareFolder = L"80";

	//Copy to main path (remote)
	{
		impersonateConnection(sFilenameInShareFolder);
		if (IsEnoughSpace(m_settings.RemoteSavePath, m_settings.DiskLimit)) 		{
			BOOL Movied = MoveFile(info.FileName.c_str(), sFilenameInShareFolder.c_str());
			if (Movied)
			{
				writeLog(info, sFilenameInShareFolder, true);
				return;
			}
			else
			{
				/*DWORD dwErr = ::GetLastError();

				if (dwErr != m_lastCopyingError)
				{
					m_lastCopyingError = dwErr;
					laLogError("MoveRecord::Unable to move {%s} to {%s} Code %u",
						info.FileName.c_str(), sFilenameInShareFolder.c_str(), dwErr);
				}

				writeLog(info, GetStringError(dwErr), false);*/
			}
		}
		else 		{
			LogError("MoveRecord::No free space on remote path", 0);
		}
	}	
	//Copy to local storage
	{
		std::wstring sFilenameInShareFolder = L"80";
		BOOL Movied = MoveFile(info.FileName.c_str(), sFilenameInShareFolder.c_str());
		if (Movied)
		{
			//createMetaFile(info, sFilenameInShareFolder);
			return;
		}
		else
		{
			DWORD dwErr = ::GetLastError();

			/*if (dwErr != m_lastCopyingError)
			{
				m_lastCopyingError = dwErr;
				laLogError("MoveLocal::Unable to move {%s} to {%s}. Code %u",
					info.FileName.c_str(), sFilenameInShareFolder.c_str(), dwErr);
			}*/
		}
	}
	DeleteFile(info.FileName.c_str());
}

bool CEventLogger::moveLocalRecord(const TActualLogEvent& info)
{
	std::wstring sFilenameInShareFolder = L"80";

	impersonateConnection(sFilenameInShareFolder);
	if (IsEnoughSpace(m_settings.RemoteSavePath, m_settings.DiskLimit)) 	{
		BOOL Movied = MoveFile(info.FileName.c_str(), sFilenameInShareFolder.c_str());
		if (Movied)
		{
			writeLog(info, sFilenameInShareFolder, true);
			return true;
		}
		else
		{
			DWORD dwCode = ::GetLastError();
			if (dwCode == ERROR_FILE_NOT_FOUND)
				return true;

		/*	if (dwCode != m_lastCopyingError)
			{
				m_lastCopyingError = dwCode;
				laLogError("MoveLocalRecord::Unable to move {%s} to {%s} Code %d",
					info.FileName.c_str(), sFilenameInShareFolder.c_str(), dwCode);
			}*/
		}
	}
	else 	{
		LogError("MoveLocalRecord::No free space on remote path", 0);
	}
	return false;
}

void CEventLogger::writeLog(const TActualLogEvent& info, const std::wstring& sFilename, bool copied)
{
	
}
