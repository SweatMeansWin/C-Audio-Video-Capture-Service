#pragma once

#include <wtypes.h>
#include "Logging.h"

enum class RegDataType : uint16_t {
	Unknown,
	String,
	ExpandString,
	DWORD,
	QWORD,
	Binary,
};

struct TRegKeyInfo {
	DWORD NumSubKeys;
	DWORD MaxSubKeyLen;
	DWORD NumValues;
	DWORD MaxValueLen;
	DWORD MaxDataLen;
	FILETIME LastWriteTime;
};

struct TRegDataInfo {
	RegDataType DataType;
	DWORD DataSize;
	TRegDataInfo(TRegDataInfo&& info) = default;
};

class CWinRegistryClient
{
public:
	CWinRegistryClient(DWORD aAccess = KEY_ALL_ACCESS);
	~CWinRegistryClient();

	CWinRegistryClient(CWinRegistryClient& client) = delete;
	CWinRegistryClient(CWinRegistryClient&& client) = default;

	DWORD GetLastError() const { return m_dwLastError; };

	HKEY getRootKey() const { return m_hkRootKey; };
	void setRootKey(HKEY aValue);

	bool OpenKey(const std::wstring& aKey, bool aCanCreate=false);
	void CloseKey();

	bool WriteString(const std::wstring& aName, const std::wstring& aValue);
	bool WriteDWORD(const std::wstring& aName, DWORD aValue);

	bool GetStringSize(const std::wstring& aName, PDWORD aStrBufferSize);

	bool ReadString(const std::wstring& aName, LPWSTR aStrBuffer, DWORD aStrBufferSize);
	bool ReadString(const std::wstring& Name, std::wstring& ValueOut);
	bool ReadDWORD(const std::wstring& aName, PDWORD aValue);
private:
	std::wstring m_path;
	bool m_bCloseRootKey;
	HKEY m_hkCurrentKey;
	HKEY m_hkRootKey;
	DWORD m_dwAccess;
	DWORD m_dwLastError;

	void changeKey(HKEY aValue, const std::wstring& aPath);
	bool checkResult(DWORD aRes);

	bool putData(const std::wstring& aName, const PBYTE aBuffer, DWORD aBuffSize, RegDataType aRegDataType);
	bool getData(const std::wstring& aName, PVOID aBuffer, DWORD aBufferSize);

	DWORD regDataToDataType(RegDataType aRegDataType);
	RegDataType dataTypeToRegData(DWORD aDataType);

	bool getKeyInfo(TRegKeyInfo& apKeyInfo);
	bool getDataInfo(const std::wstring& aName, TRegDataInfo& aDataInfo);
};
