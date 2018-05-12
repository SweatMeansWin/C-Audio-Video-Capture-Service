#pragma once

#include <exception>

struct BaseError : std::exception {
	BaseError(const std::string& error_msg);
	BaseError(const std::string& error_msg, uint32_t error_code);
};
struct GraphInitializationError : public BaseError {
	GraphInitializationError(const std::string& error_msg, uint32_t error_code) noexcept
		: BaseError(error_msg, error_code) {}
};
struct NoDeviceFound : public BaseError {
	NoDeviceFound() noexcept : BaseError("No devices found in system") {}
	NoDeviceFound(const std::string& error_msg) noexcept : BaseError(error_msg) {}
};
struct DeviceIsBusyError : public BaseError {
	DeviceIsBusyError() noexcept : BaseError("Chosen device is busy") {}
	DeviceIsBusyError(const std::string& error_msg) noexcept : BaseError(error_msg) {}
};
struct MediaControlError : public BaseError {
	MediaControlError(const std::string& error_msg, uint32_t error_code=0) noexcept
		: BaseError(error_msg, error_code) {}
};
