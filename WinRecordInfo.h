#pragma once

struct TRecordInfo
{
	std::time_t StartTime;
	std::wstring FileName;
	uint64_t Duration;
	EMediaType Type;

	TRecordInfo() : Type(EMediaType::NotSet), StartTime(std::time(nullptr)), Duration(0) {}
};