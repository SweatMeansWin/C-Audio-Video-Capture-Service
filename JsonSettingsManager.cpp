#include "PreCompiled.h"
#include "JsonSettingsManager.h"

#include "Json.hpp"
#include "Logging.h"

#include <fstream>

using namespace nlohmann;

void CJsonSettingsManager::Update()
{
	LogMessage("Settings::Json::Update");

	std::ifstream ifs("f");
	if (!ifs.is_open()) {
		throw std::ios::failure("Unable to open json config fail");
	}
	json j;
	ifs >> j;

	p_settings.DurationSec = j["Media"]["Duration"];
	p_settings.TimeOutSec = j["Media"]["TimeoOut"];
	p_settings.Repeatable = true;

	p_settings.audioSettings.SetName(j["Media"]["Audio"]["DeviceName"]);
	p_settings.audioSettings.Quality = j["Media"]["Audio"]["Quality"];
	p_settings.audioSettings.Enabled = j["Media"]["Audio"]["Enabled"];

	p_settings.videoSettings.SetName(j["Media"]["Video"]["DeviceName"]);
	p_settings.videoSettings.Quality = j["Media"]["Video"]["Quality"];
	p_settings.videoSettings.Enabled = j["Media"]["Video"]["Enabled"];
}
