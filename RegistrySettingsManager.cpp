#include "PreCompiled.h"
#include "RegistrySettingsManager.h"
#include "Logging.h"
#include "WinRegistryClient.h"
#include "WinRegistryConstants.h"
#include "GOST.h"

DWORD getDWORDFromRegistry(CWinRegistryClient& client, LPCWSTR path) {
	DWORD buffer = 0;
	client.ReadDWORD(path, &buffer);
	return buffer;
}

std::wstring getDecryptedStringFromRegistry(CWinRegistryClient& client, LPCWSTR path) {
	DWORD dwBufferSize = 0;
	std::wstring descryptedString;
	if (client.GetStringSize(REG_MEDIA_SHARE_PASS, &dwBufferSize) && dwBufferSize > sizeof(TCHAR)) {
		auto buffer = new WCHAR[dwBufferSize];
		if (client.ReadString(path, buffer, dwBufferSize)) {
			// Remove \n
			dwBufferSize -= sizeof(WCHAR);
			if (dwBufferSize % 8 == 0) {
				if (GOST::Decrypt(reinterpret_cast<uint32_t*>(buffer), dwBufferSize)) {
					descryptedString = buffer;
				}
			}
		}
		delete[] buffer;
	}
	return descryptedString;
}

CWinRegistryClient getClient(const std::wstring& keyPath) {
	CWinRegistryClient regClient(KEY_ALL_ACCESS | KEY_WOW64_64KEY);
	regClient.setRootKey(HKEY_LOCAL_MACHINE);
	regClient.OpenKey(SERVICE_REG_PATH);
	return regClient;
}

void CRegistrySettingsManager::Update()
{
	LogMessage("Settings::Registry::Update");

	p_settings.Clear();
	updateDevices();
	updateSaveInfo();
}

void CRegistrySettingsManager::updateDevices()
{
	auto regClient = getClient(SERVICE_REG_PATH);
	if (regClient.GetLastError() == ERROR_SUCCESS) {

		p_settings.DurationSec = getDWORDFromRegistry(regClient, REG_MEDIA_DURATION);
		p_settings.TimeOutSec = getDWORDFromRegistry(regClient, REG_MEDIA_TIMEOUT);
		p_settings.Repeatable = true;

		if (regClient.OpenKey(AUDIO_SETT_REG_PATH))
		{
			regClient.ReadString(REG_MEDIA_DEVICE, p_settings.audioSettings.Name);
			p_settings.audioSettings.Quality = static_cast<ECaptureQuality>(getDWORDFromRegistry(regClient, REG_MEDIA_QUALITY));
			p_settings.audioSettings.Enabled = bool(getDWORDFromRegistry(regClient, REG_MEDIA_ENABLED));
		}
		if (regClient.OpenKey(VIDEO_SETT_REG_PATH))
		{
			regClient.ReadString(REG_MEDIA_DEVICE, p_settings.videoSettings.Name);
			p_settings.videoSettings.Quality = static_cast<ECaptureQuality>(getDWORDFromRegistry(regClient, REG_MEDIA_QUALITY));
			p_settings.videoSettings.Enabled = bool(getDWORDFromRegistry(regClient, REG_MEDIA_ENABLED));
		}
	}
}

void CRegistrySettingsManager::updateSaveInfo()
{
	auto regClient = getClient(SERVICE_REG_PATH);
	if (regClient.GetLastError() == ERROR_SUCCESS) {
		//GET SAVE DIRECTORY
		regClient.ReadString(REG_MEDIA_REMOTE_DIR, p_settings.saveSettings.RemoteSavePath);
		//value in GB, convert to bytes
		p_settings.saveSettings.DiskLimit = 
			static_cast<uint64_t>(getDWORDFromRegistry(regClient, REG_MEDIA_BYTES_LIMIT)) * 1024/*MB*/ * 1024/*KB*/ * 1024/*B*/;
		p_settings.saveSettings.RemoteLogin = getDecryptedStringFromRegistry(regClient, REG_MEDIA_SHARE_USER);
		p_settings.saveSettings.RemotePassword = getDecryptedStringFromRegistry(regClient, REG_MEDIA_SHARE_PASS);
	}
}
