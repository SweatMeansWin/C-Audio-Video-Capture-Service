#include "PreCompiled.h"
#include "Errors.h"

#include "Logging.h"


BaseError::BaseError(const std::string& error_msg) : std::exception(error_msg.c_str()) {
	LogError(error_msg, 0);
}

BaseError::BaseError(const std::string& error_msg, uint32_t error_code) : std::exception(error_msg.c_str()) {
	LogError(error_msg, error_code);
}