#pragma once

#include "CommonSettings.h"

class CScheduler
{
public:
	typedef std::function<void()> EventHandler;

	CScheduler(EventHandler pStartFunc, EventHandler pStopFunc);
	~CScheduler();

	void	Start();
	void	Stop();

	void	SetSettings(const TSchedulerSettings& Settings);

	int Timeout;
private:
	TSchedulerSettings			m_settings;

	EventHandler		m_fnStart;
	EventHandler		m_fnStop;

	std::atomic_bool			m_bChecking;
	std::atomic_bool			m_bWorking;
	TSchedulerSettings::Period	m_currentPeriod;

	void checkPeriods();
	void stopRecord();

	void check();
};
