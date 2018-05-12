#include "PreCompiled.h"
#include "Scheduler.h"

#include "Logging.h"
#include "WinHeaders.h"


enum class ESchedulerState {
	Wait,
	Progress,
};

CScheduler::CScheduler(EventHandler pStartFunc, EventHandler pStopFunc)
	: Timeout(1), m_bChecking(false), m_bWorking(false), m_fnStart(pStartFunc), m_fnStop(pStopFunc)
{
}

CScheduler::~CScheduler()
{
	Stop();
}

void CScheduler::Start()
{
	LogMessage("Schedule::Starting...");

	m_bChecking = true;
	std::thread thd(&CScheduler::check, this);
	thd.detach();
}

void CScheduler::Stop()
{
	LogMessage("Schedule::Stopping...");

	m_bChecking = false;
	stopRecord();
}

void CScheduler::SetSettings(const TSchedulerSettings & Settings) {
	m_settings = Settings;
}

ESchedulerState operator == (const SYSTEMTIME& st, const TSchedulerSettings::TScheduleTime scheduleTime) {
	SYSTEMTIME targetTime = st;
	targetTime.wHour = scheduleTime.Hour;
	targetTime.wMinute = scheduleTime.Minute;
	targetTime.wSecond = 0;
	targetTime.wMilliseconds = 0;

	FILETIME ftCurrentTime, ftTargetTime;
	SystemTimeToFileTime(&st, &ftCurrentTime);
	SystemTimeToFileTime(&targetTime, &ftTargetTime);

	return CompareFileTime(&ftCurrentTime, &ftTargetTime) == 0 ? ESchedulerState::Progress : ESchedulerState::Wait;
}

LONG cmp(const SYSTEMTIME& st, const TSchedulerSettings::TScheduleTime scheduleTime) {
	SYSTEMTIME targetTime = st;
	targetTime.wHour = scheduleTime.Hour;
	targetTime.wMinute = scheduleTime.Minute;
	targetTime.wSecond = 0;
	targetTime.wMilliseconds = 0;

	FILETIME ftCurrentTime, ftTargetTime;
	SystemTimeToFileTime(&st, &ftCurrentTime);
	SystemTimeToFileTime(&targetTime, &ftTargetTime);

	return CompareFileTime(&ftCurrentTime, &ftTargetTime);
}

ESchedulerState work(const SYSTEMTIME& st, const TSchedulerSettings::Period &p)
{
	return cmp(st, p.first) > -1 && cmp(st, p.second) < 1 ? ESchedulerState::Progress : ESchedulerState::Wait;
}

void CScheduler::checkPeriods()
{
	if (m_settings.Periods.size())
	{
		SYSTEMTIME currentTime;
		GetLocalTime(&currentTime);

		switch (work(currentTime, m_currentPeriod)) {
		case ESchedulerState::Wait:
			// Means our period has ended
			stopRecord();
			break;
		case ESchedulerState::Progress:
			return;
		}

		// Looking for new
		for (auto const& it : m_settings.Periods[currentTime.wDayOfWeek]) {
			switch (work(currentTime, it)) {
			case ESchedulerState::Wait:
				continue;
			case ESchedulerState::Progress:
				LogDebug("Schedule::Enter the period. Start record");

				m_fnStart();
				m_bWorking = true;
				m_currentPeriod = it;
			}
			break;
		}
	}
}

void CScheduler::stopRecord()
{
	m_fnStop();
}

void CScheduler::check()
{
	do {
		checkPeriods();
		std::this_thread::sleep_for(std::chrono::seconds(Timeout));
	} while (m_bChecking);
}
