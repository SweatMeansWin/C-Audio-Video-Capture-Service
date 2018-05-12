#pragma once

#include <atomic>

#include "CommonSettings.h"
#include "Errors.h"


class CBaseCaptureGraph {
public:
	virtual ~CBaseCaptureGraph() = default;
	virtual bool Create() = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;

	void SetSettings(const TCaptureSettings &settings) { p_settings = settings; }
	TCaptureSettings *const GetSettings() { return &p_settings; }

	virtual EMediaType GetType() = 0;

protected:
	TCaptureSettings p_settings;
};
