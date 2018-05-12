//
// SweatMeansWin 22.05.2018
//

#include "BaseWinCaptureGraph.h"
#include "RegistrySettingsManager.h"
#include "JsonSettingsManager.h"
#include "Scheduler.h"
#include "CommonSettings.h"

class AVController {
public:
	AVController();
	~AVController();

	bool Init();
	void Deinit();

	uint64_t StartRecord(const TCaptureSettings& settings);
	void StopRecord(uint64_t identifier);
	void StopAll();
	
	void UpdateSettings();
	bool GetActiveRecords(std::vector<TCaptureInfo>& records);
	bool GetDeviceList(TDevices& devices);

private:
	std::atomic_bool m_bInit;
	std::mutex m_mutex;
	
	std::map<uint64_t, std::pair<TCaptureInfo, std::shared_ptr<CBaseCaptureGraph>>> m_graphs;
	// CRegistrySettingsManager m_settingsManager;
	CJsonSettingsManager m_settingsManager;
	//CScheduler m_scheduler;

	void initSystem();
	uint64_t generateThreadId();
	std::shared_ptr<CBaseCaptureGraph> constructGraph(EMediaType mediaType);

	static void graphWorker(std::shared_ptr<CBaseCaptureGraph> pGraph);
};
