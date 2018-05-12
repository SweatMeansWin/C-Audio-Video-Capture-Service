#include "PreCompiled.h"
#include "Tests.h"

#include "AVController.h"
#include "Logging.h"

#include <algorithm>

std::ostream& operator <<(std::ostream& os, EMediaType mt) {
	switch (mt)
	{
	case EMediaType::Audio:
		os << "Audio";
		break;
	case EMediaType::Video:
		os << "Video";
		break;
	case EMediaType::AudioVideo:
		os << "AudioVideo";
		break;
	case EMediaType::Photo:
		os << "Photo";
		break;
	case EMediaType::Online:
		os << "Online";
		break;
	default:
		break;
	}
	return os;
}

std::ostream& operator <<(std::ostream& os, TCaptureInfo tci) {
	os << "CaptureId=" << tci.CaptureId << ", Graph=" << tci.GraphType << ", Start=" << tci.StartTime;
	return os;
}

template <typename T>
std::ostream& operator <<(std::ostream& os, std::vector<T> tciv) {
	os << "vec[" << tciv.size() << "] {";
	for (size_t idx = 0; idx < tciv.size(); ++idx) {
		os << tciv.at(idx);
		if (idx < tciv.size() - 1)
			os << ", ";
	}
	os << "}";
	return os;
}

void subtest(AVController& ctrl, TCaptureSettings& settings, bool audioEnabled, bool videoEnabled, EMediaType graphType) {
	std::cout << " [" << graphType << "]" << std::endl;
	std::cout << "Audio: " << audioEnabled << std::endl;
	std::cout << "Video: " << videoEnabled << std::endl;
	settings.GraphType = graphType;
	settings.audioSettings.Enabled = audioEnabled;
	settings.videoSettings.Enabled = videoEnabled;

	std::vector<TCaptureInfo> infoVec;

	uint64_t gid = ctrl.StartRecord(settings);
	if (gid > 0) {
		ctrl.GetActiveRecords(infoVec);
		std::cout << infoVec << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(settings.DurationSec));
		ctrl.StopRecord(gid);
	}
};

void failed_test(AVController& ctrl, TCaptureSettings& settings) {
	try {
		subtest(ctrl, settings, false, false, EMediaType::AudioVideo);
		throw "should failed";
	}
	catch (...) {}
	try {
		subtest(ctrl, settings, false, false, EMediaType::Online);
		throw "should failed";
	}
	catch (...) {}
	subtest(ctrl, settings, true, true, EMediaType::NotSet);
}

void video_test(AVController& ctrl, TCaptureSettings& settings) {
	subtest(ctrl, settings, false, true, EMediaType::Video);
	subtest(ctrl, settings, false, true, EMediaType::AudioVideo);
	subtest(ctrl, settings, false, true, EMediaType::Photo);
	subtest(ctrl, settings, false, true, EMediaType::Online);
}

void audio_test(AVController& ctrl, TCaptureSettings& settings) {
	subtest(ctrl, settings, true, false, EMediaType::Audio);
	subtest(ctrl, settings, true, true, EMediaType::AudioVideo);
	subtest(ctrl, settings, true, true, EMediaType::Online);
	subtest(ctrl, settings, true, false, EMediaType::Online);
	subtest(ctrl, settings, true, true, EMediaType::NotSet);
}

void full_test(AVController& ctrl, TCaptureSettings& settings) {
	audio_test(ctrl, settings);
	video_test(ctrl, settings);
	failed_test(ctrl, settings);
}

void Test() {
	AVController ctrl;
	ctrl.Init();

	TDevices dec;
	if (!ctrl.GetDeviceList(dec) || dec.empty()) {
		throw NoDeviceFound();
	}

	TCaptureSettings settings;
	settings.DurationSec = 5;
	settings.saveSettings.LocalSavePath = L".";

	auto it = std::find_if(dec.begin(), dec.end(), [](TDeviceInfo& info) { return info.Type == EDeviceType::Video; });
	if (it != dec.end())
		settings.videoSettings.Name = it->Name;
	auto rit = std::find_if(dec.begin(), dec.end(), [](TDeviceInfo& info) { return info.Type == EDeviceType::Audio; });
	if (rit != dec.end())
		settings.audioSettings.Name = rit->Name;
	settings.Repeatable = false;

	full_test(ctrl, settings);

	ctrl.Deinit();
}
