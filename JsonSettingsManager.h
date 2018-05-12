#pragma once

#include "BaseSettingsManager.h"
#include "CommonSettings.h"

class CJsonSettingsManager :
	public CBaseSettingsManager<TCaptureSettings>
{
public:
	virtual void Update() override;
};

