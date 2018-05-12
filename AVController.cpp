//
// Created by evgen on 09.05.2018.
//

#include "PreCompiled.h"
#include "AVController.h"

#include "WinAudioVideoCaptureGraph.h"
#include "WinOnlineCaptureGraph.h"
#include "WinPhotoCaptureGraph.h"
#include "DeviceEnumerator.h"
#include "SettingsValidator.h"
#include "Logging.h"

#include <random>

// randomize
std::mt19937_64 m_e2;

AVController::AVController() : m_bInit(false) {
	CoInitialize(nullptr);

	std::random_device rd;
	m_e2 = std::mt19937_64(rd());
}

AVController::~AVController() {
	Deinit();
	CoUninitialize();
}

bool AVController::Init() {
	if (m_bInit)
		return true;

	//    m_pScheduler = new ÑTimeScheduler(&startScheduleRecord, &stopScheduleRecord);
	//
	initSystem();

	LogMessage("AVController initialized");
	m_bInit = true;
	return m_bInit;
}

uint64_t AVController::generateThreadId() {
	return m_e2();
}

void AVController::initSystem() {
	try {
		m_settingsManager.Update();
	}
	catch (std::exception& exc) {
		LogError(exc.what(), 0);
	}
}

std::shared_ptr<CBaseCaptureGraph> AVController::constructGraph(EMediaType mediaType) {
	switch (mediaType)
	{
	case EMediaType::Audio:
	case EMediaType::Video:
	case EMediaType::AudioVideo:
		return std::make_shared<CWinAudioVideoCaptureGraph>();
	case EMediaType::Photo:
		return std::make_shared<CWinPhotoCaptureGraph>();
	case EMediaType::Online:
		return std::make_shared<CWinOnlineCaptureGraph>();
	case EMediaType::NotSet:
	default:
		break;
	}
	return nullptr;
}

void AVController::Deinit() {
	if (!m_bInit)
		return;

	StopAll();

	LogMessage("AVController deinitialized");

	m_bInit = false;
}

void AVController::StopAll() {
	std::lock_guard<std::mutex> lck(m_mutex);

	std::vector<uint64_t> graphIds;
	for (auto& it : m_graphs) {
		graphIds.push_back(it.first);
	}
	for (uint64_t gid : graphIds) {
		StopRecord(gid);
	}
}

bool AVController::GetActiveRecords(std::vector<TCaptureInfo>& records) {
	std::lock_guard<std::mutex> lck(m_mutex);

	if (m_graphs.empty())
		return false;

	for (auto it = m_graphs.cbegin(); it != m_graphs.cend(); ++it) {
		records.push_back(it->second.first);
	}
	return true;
}

bool AVController::GetDeviceList(TDevices& devices) {
	try {
		devices = CDeviceEnumerator().GetDevices();
	}
	catch (std::exception& exc) {
		LogError(exc.what(), 0);
		return false;
	}
	return true;
}

uint64_t AVController::StartRecord(const TCaptureSettings & settings) {
	CSettingsValidator::Validate(settings);
	//Create graph
	auto pCaptureGraph = constructGraph(settings.GraphType);
	if (!pCaptureGraph) {
		return 0;
	}
	// Insert settings
	pCaptureGraph->SetSettings(settings);
	// Check creating process status
	bool created = pCaptureGraph->Create();
	if (created) {
		TCaptureInfo captureInfo;
		captureInfo.GraphType = pCaptureGraph->GetType();

		std::thread thd(&AVController::graphWorker, pCaptureGraph);
		thd.detach();
		
		uint64_t gid = generateThreadId();
		std::lock_guard<std::mutex> lck(m_mutex);

		captureInfo.CaptureId = gid;
		m_graphs[gid] = std::make_pair(captureInfo, std::move(pCaptureGraph));
		return gid;
	}
	return 0;
}

void AVController::StopRecord(uint64_t identifier) {
	std::lock_guard<std::mutex> lck(m_mutex);

	auto it = m_graphs.find(identifier);
	if (it == m_graphs.cend()) {
		return;
	}
	// Stop and wait graph
	it->second.second->Stop();
	// Remove
	m_graphs.erase(it);
}

void AVController::UpdateSettings() {
	m_settingsManager.Update();
}

void AVController::graphWorker(std::shared_ptr<CBaseCaptureGraph> pGraph) {
	try {
		pGraph->Start();
	}
	catch (BaseError) {
		std::stringstream ss;
		ss << "Thread with id [" << std::this_thread::get_id() << "] stopped with exception";
		LogError(ss.str(), 0);
	}
}

