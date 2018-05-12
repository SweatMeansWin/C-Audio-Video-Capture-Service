#pragma once

#include "BaseSettingsManager.h"
#include "CommonSettings.h"

class CRegistrySettingsManager : public CBaseSettingsManager<TCaptureSettings>
{
public:
	virtual void Update() override;
private:
	void updateDevices();
	void updateSaveInfo();
};
