#pragma once

#include "DeviceSettings.h"

enum class EMediaType : uint16_t {
	Audio,
	Video,
	AudioVideo,
	Photo,
	Online,
	NotSet
};

struct TSystemSettings
{
	std::wstring ProfileDirectory;
	std::wstring SystemLogFilePath;

	void Clear() {
		ProfileDirectory.clear();
		SystemLogFilePath.clear();
	}
};

struct TSaveSettings
{
	std::wstring LocalSavePath;
	// Настройки удалённого подключения
	std::wstring RemoteSavePath;
	std::wstring RemoteLogin;
	std::wstring RemotePassword;
	// Предел свободного места в папке назначения, когда прекращаем запись
	uint64_t DiskLimit;

	TSaveSettings() noexcept : DiskLimit(0) {}

	void Clear() {
		DiskLimit = 0;
		RemoteSavePath.clear();
		RemotePassword.clear();
		LocalSavePath.clear();
		RemoteLogin.clear();
	}
};

struct TSchedulerSettings
{
	struct TScheduleTime {
		uint16_t Hour;
		uint16_t Minute;
	};
	typedef std::pair<TScheduleTime, TScheduleTime> Period;
	typedef std::vector<Period> DaySchedule;
	typedef long DayOfWeek;  // SYSTEMTIME::wDayOfWeek

	std::map<DayOfWeek, DaySchedule> Periods;
	bool Enabled;

	TSchedulerSettings() noexcept : Enabled(false) { }

	void Clear() {
		Enabled = false;
		Periods.clear();
	}
};

struct TCaptureSettings
{
	TDeviceInfo audioSettings;
	TDeviceInfo videoSettings;
	TSaveSettings saveSettings;
	TSystemSettings systemSettings;
	TSchedulerSettings schedulerSettings;

	EMediaType GraphType;
	bool Repeatable;
	long DurationSec;
	long TimeOutSec;

	TCaptureSettings() noexcept : GraphType(EMediaType::NotSet), Repeatable(false), DurationSec(0), TimeOutSec(0) {}

	void Clear() {
		audioSettings.Clear();
		videoSettings.Clear();
		saveSettings.Clear();
		GraphType = EMediaType::NotSet;
		Repeatable = false;
		DurationSec = 0;
		TimeOutSec = 0;
	}
};

struct TCaptureInfo
{
	std::time_t StartTime;
	EMediaType GraphType;
	uint64_t CaptureId;

	TCaptureInfo() : StartTime(std::time(nullptr)), GraphType(EMediaType::NotSet), CaptureId(0) { }
};
