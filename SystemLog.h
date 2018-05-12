#pragma once

#include <map>

enum class SystemLogVersion {
	Default,
	v0_1,
	v1_0,
};

struct TLogEventv0_1 {
	const SystemLogVersion version = SystemLogVersion::v0_1;

	std::wstring FileName;
	std::time_t EventTime;
	uint16_t EventType;
	
	TLogEventv0_1() : EventType(0), EventTime() {}
};

using TActualLogEvent = TLogEventv0_1;
