#include "PreCompiled.h"
#include "SettingsValidator.h"

#include "Logging.h"

std::ostream & operator<<(std::ostream & os, const ECaptureQuality q)
{
	switch (q)
	{
	case ECaptureQuality::Low: os << "Low";
	case ECaptureQuality::Medium: os << "Medium";
	case ECaptureQuality::High: os << "High";
	}
	return os;
}

void CSettingsValidator::Validate(const TCaptureSettings & settings)
{
	logSettings(settings);
	// Warning first
	std::vector<std::string> issues;
	if (settings.saveSettings.RemoteSavePath.empty()) {
		issues.push_back("Remote save path missed");
	}
	if (settings.TimeOutSec > CSettingsValidator::MAX_TIMEOUT) {
		issues.push_back("Timeout value is big. Just pay attention = " + std::to_string(settings.TimeOutSec));
	}
	if (settings.saveSettings.DiskLimit == 0) {
		issues.push_back("Disk limit missed. May destroy all disk space");
	}
	if (settings.systemSettings.SystemLogFilePath.empty()) {
		issues.push_back("System log file path missed");
	}
	if (settings.systemSettings.ProfileDirectory.empty()) {
		issues.push_back("Profile directory missed");
	}
	if (settings.saveSettings.RemoteSavePath.empty()) {
		issues.push_back("Remote save path missed");
	}
	if (settings.saveSettings.RemoteLogin.empty()) {
		issues.push_back("Remote login missed");
	}
	if (settings.saveSettings.RemotePassword.empty()) {
		issues.push_back("Remote password missed");
	}
	// Check result
	if (!issues.empty()) {
		std::stringstream ss;
		for (size_t idx = 0; idx < issues.size(); ++idx)
			LogWarning("Validator::" + issues.at(idx));
	}
	issues.clear();
	// Critical errors, we can't work without these params
	if (!settings.audioSettings.Enabled && !settings.videoSettings.Enabled) {
		issues.push_back("Audio and video disabled");
	}
	if (settings.saveSettings.LocalSavePath.empty()) {
		issues.push_back("Local save path missed");
	}
	if (!issues.empty()) {
		std::stringstream ss;
		for (size_t idx = 0; idx < issues.size(); ++idx) {
			ss << "Validator::" << issues.at(idx);
			if (idx < issues.size() - 1)
				ss << std::endl;
		}
		throw SettingsValidationError(ss.str());
	}
}

void logDevicefn(const TDeviceInfo& deviceSettings, const char* deviceType) {
	LogDebugStream << "Device::" << deviceType;
	LogDebugStream << "Name: " << deviceSettings.Name;
	LogDebugStream << "Enabled: " << deviceSettings.Enabled;
	LogDebugStream << "Quality: " << deviceSettings.Quality;
}

void CSettingsValidator::logSettings(const TCaptureSettings & settings)
{
	LogDebugStream << "Settings::Registry";
	// System
	LogDebugStream << "SystemLogFilePath: " << settings.systemSettings.SystemLogFilePath;
	LogDebugStream << "ProfileDirectory: " << settings.systemSettings.ProfileDirectory;
	// Save local
	LogDebugStream << "RemoteSavePath: " << settings.saveSettings.RemoteSavePath;
	LogDebugStream << "LocalSavePath: " << settings.saveSettings.LocalSavePath;
	LogDebugStream << "DiskLimit: " << settings.saveSettings.DiskLimit;
	// Sensitive info
	LogDebugStream << "Login" << settings.saveSettings.RemoteLogin;
	LogDebugStream << "Password" << settings.saveSettings.RemotePassword;
	// Devices
	logDevicefn(settings.audioSettings, "Audio");
	logDevicefn(settings.videoSettings, "Video");
}
