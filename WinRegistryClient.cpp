#include "PreCompiled.h"
#include "WinRegistryClient.h"


CWinRegistryClient::CWinRegistryClient(DWORD aAccess) :
	m_hkRootKey(HKEY_CURRENT_USER),
	m_dwLastError(0),
	m_dwAccess(aAccess),
	m_hkCurrentKey(NULL),
	m_bCloseRootKey(false)
{ }

CWinRegistryClient::~CWinRegistryClient() {
	CloseKey();
}

void CWinRegistryClient::setRootKey(HKEY aValue)
{
	if (aValue != m_hkRootKey)
	{
		if (m_bCloseRootKey)
		{
			RegCloseKey(m_hkRootKey);
			m_bCloseRootKey = false;
		}

		m_hkRootKey = aValue;
		CloseKey();
	}
}

void CWinRegistryClient::CloseKey()
{
	if (m_hkCurrentKey != 0)
	{
		RegFlushKey(m_hkCurrentKey);
		RegCloseKey(m_hkCurrentKey);

		m_hkCurrentKey = 0;
		m_path.clear();
	}
}

bool CWinRegistryClient::OpenKey(const std::wstring& aKey, bool aCanCreate)
{
	bool result = false;
	HKEY tempKey = 0;
	DWORD disposition = 0;

	if (!aCanCreate)
	{
		result = checkResult(RegOpenKeyEx(m_hkRootKey, aKey.c_str(), 0, m_dwAccess, &tempKey));
	}
	else
	{
		result = checkResult(
			RegCreateKeyEx(m_hkRootKey, aKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, m_dwAccess, NULL, &tempKey, &disposition)
		);
	}

	if (result)
		changeKey(tempKey, aKey);

	return result;
}

bool CWinRegistryClient::checkResult(DWORD aRes)
{
	m_dwLastError = aRes;
	return (aRes == ERROR_SUCCESS);
}

void CWinRegistryClient::changeKey(HKEY aValue, const std::wstring& aPath)
{
	CloseKey();

	m_hkCurrentKey = aValue;
	m_path = aPath;
}

bool CWinRegistryClient::WriteString(const std::wstring& aName, const std::wstring& aValue)
{
	return putData(aName, (PBYTE)aValue.c_str(), aValue.size() * sizeof(WCHAR), RegDataType::String);
}

bool CWinRegistryClient::putData(const std::wstring& aName, const PBYTE aBuffer, DWORD aBuffSize, RegDataType aRegData)
{
	return checkResult(
		RegSetValueEx(m_hkCurrentKey, aName.c_str(), 0, regDataToDataType(aRegData), aBuffer, aBuffSize)
	);
}

DWORD CWinRegistryClient::regDataToDataType(RegDataType aTDataType)
{
	switch (aTDataType)
	{
	case RegDataType::String: return REG_SZ;
	case RegDataType::ExpandString: return REG_EXPAND_SZ;
	case RegDataType::DWORD: return REG_DWORD;
	case RegDataType::QWORD: return REG_QWORD;
	case RegDataType::Binary: return REG_BINARY;
	default: return REG_NONE;
	}
}

RegDataType CWinRegistryClient::dataTypeToRegData(DWORD aDataType)
{
	switch (aDataType)
	{
	case REG_SZ: return RegDataType::String;
	case REG_EXPAND_SZ: return RegDataType::ExpandString;
	case REG_DWORD: return RegDataType::DWORD;
	case REG_QWORD: return RegDataType::QWORD;
	case REG_BINARY: return RegDataType::Binary;
	default: return RegDataType::Unknown;
	}
}

bool CWinRegistryClient::WriteDWORD(const std::wstring& aName, DWORD aValue)
{
	return putData(aName, (PBYTE)&aValue, sizeof(DWORD), RegDataType::DWORD);
}

bool CWinRegistryClient::getKeyInfo(TRegKeyInfo& apKeyInfo)
{
	memset(&apKeyInfo, 0, sizeof(TRegKeyInfo));

	return checkResult(
		RegQueryInfoKey(
			m_hkCurrentKey,
			NULL,
			NULL,
			NULL,
			&apKeyInfo.NumSubKeys,
			&apKeyInfo.MaxSubKeyLen,
			NULL,
			&apKeyInfo.NumValues,
			&apKeyInfo.MaxValueLen,
			&apKeyInfo.MaxDataLen,
			NULL,
			&apKeyInfo.LastWriteTime
		));
}

bool CWinRegistryClient::GetStringSize(const std::wstring& aName, PDWORD aStrBufferSize)
{
	TRegDataInfo dataInfo = { RegDataType::Unknown, 0 };

	if (getDataInfo(aName, dataInfo))
	{
		*aStrBufferSize = dataInfo.DataSize;
		return true;
	};

	return false;
}

bool CWinRegistryClient::ReadString(const std::wstring& aName, LPWSTR aStrBuffer, DWORD aStrBufferSize)
{
	TRegDataInfo dataInfo = { RegDataType::Unknown, 0 };

	if (getDataInfo(aName, dataInfo))
	{
		if (dataInfo.DataSize > 0 && dataInfo.DataSize <= aStrBufferSize
			&& (dataInfo.DataType == RegDataType::String || dataInfo.DataType == RegDataType::ExpandString)
			) {
			return getData(aName, aStrBuffer, aStrBufferSize);
		}
	}

	return false;
}

bool CWinRegistryClient::ReadString(const std::wstring & Name, std::wstring & ValueOut)
{
	DWORD size = 0;
	if (GetStringSize(Name.c_str(), &size)) {
		ValueOut.resize(size / sizeof(TCHAR));

		if (ReadString(Name.c_str(), &*ValueOut.begin(), size)) {
			return true;
		}
	}

	return false;
}

bool CWinRegistryClient::ReadDWORD(const std::wstring& aName, PDWORD aValue)
{
	TRegDataInfo dataInfo = { RegDataType::Unknown, 0 };

	if (getDataInfo(aName, dataInfo) && dataInfo.DataType == RegDataType::DWORD) {
		return getData(aName, aValue, sizeof(DWORD));
	}

	return false;
}

bool CWinRegistryClient::getDataInfo(const std::wstring& aName, TRegDataInfo& aDataInfo)
{
	DWORD dwDataType = REG_NONE;
	ZeroMemory(&aDataInfo, sizeof(TRegDataInfo));

	bool bRes = RegQueryValueEx(m_hkCurrentKey, aName.c_str(), NULL, &dwDataType, NULL, &aDataInfo.DataSize) == ERROR_SUCCESS;

	aDataInfo.DataType = dataTypeToRegData(dwDataType);

	return bRes;
}

bool CWinRegistryClient::getData(const std::wstring& aName, PVOID aBuffer, DWORD aBufferSize)
{
	DWORD dwDataType;

	return checkResult(
		RegQueryValueEx(m_hkCurrentKey, aName.c_str(), NULL, &dwDataType, (LPBYTE)aBuffer, &aBufferSize)
	);
}