#pragma once

template <typename SettingStructure>
class CBaseSettingsManager
{
public:
	virtual void Update() = 0;
	const SettingStructure & GetSettings() { return p_settings; }
protected:
	SettingStructure p_settings; 
};

