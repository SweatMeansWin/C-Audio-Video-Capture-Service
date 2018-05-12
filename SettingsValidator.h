#pragma once

#include "CommonSettings.h"
#include "Errors.h"


struct SettingsValidationError : public BaseError {
	SettingsValidationError(const std::string& error_msg, uint32_t error_code = 0) noexcept
		: BaseError(error_msg, error_code) {}
};

class CSettingsValidator
{
public:
	static void Validate(const TCaptureSettings& settings);

private:
	static void logSettings(const TCaptureSettings& settings);
	static const uint32_t MAX_TIMEOUT = 60 * 60 * 24;	// 1 day (very suspicious)
};

