#include "PreCompiled.h"
#include "BaseWinCaptureGraph.h"

#include "Logging.h"

const wchar_t* WMV_EXTENSION = L".wmv";
const wchar_t* WAV_EXTENSION = L".wav";
const wchar_t* JPG_EXTENSION = L".jpg";

CBaseWinCaptureGraph::CBaseWinCaptureGraph()
	: p_pFilterGraph(nullptr), p_pCaptureGraphBuilder(nullptr), p_pMediaControl(nullptr), p_pMediaEvent(nullptr),
	p_pProfile(nullptr), p_pAudioSrc(nullptr), p_pVideoSrc(nullptr), p_pFileSink(nullptr),
	p_bCapturing(false), p_bShouldWork(false)
{
}

CBaseWinCaptureGraph::~CBaseWinCaptureGraph() {
	Stop();
	freeFilters();
}

bool CBaseWinCaptureGraph::Create()
{
	try
	{
		initFilters();
		checkDevice();
	}
	catch (BaseError) {
		return false;
	}
	return true;
}

void CBaseWinCaptureGraph::Start()
{
	if (p_bCapturing)
		return;

	TRecordInfo info;
	info.Type = GetType();
	do
	{
		info.StartTime = std::time(nullptr);
		info.FileName = getRecordFileName();
		LogDebug(info.FileName);
		
		process(info);

		info.Duration = static_cast<uint64_t>(std::difftime(std::time(nullptr), info.StartTime));
		// Should we ever repeat and sleep
		if (p_bShouldWork && p_settings.Repeatable && p_settings.TimeOutSec > 0) {
			std::this_thread::sleep_for(std::chrono::seconds(p_settings.TimeOutSec));
		}
		// It's timeout ago, shoud we then
	} while (p_bShouldWork && p_settings.Repeatable);

	// Notify we ended
	p_bCapturing = false;
}

void CBaseWinCaptureGraph::Stop()
{
	if (p_bCapturing)
		stop();
}

void CBaseWinCaptureGraph::createBaseGraph() {
	HRESULT hr = CoCreateInstance(
		CLSID_CaptureGraphBuilder2,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2,
		(void **)&p_pCaptureGraphBuilder);
	if (hr == CO_E_NOTINITIALIZED) {
		throw MediaControlError("CoInitialize has not been called");
	}
	raiseIfFailed(hr, "createBaseGraph::CoCreateInstance::CLSID_CaptureGraphBuilder2");

	hr = CoCreateInstance(CLSID_FilterGraph, nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder,
		(void **)&p_pFilterGraph);
	raiseIfFailed(hr, "createBaseGraph::CoCreateInstance::IID_IFilterGraph2");

	hr = p_pCaptureGraphBuilder->SetFiltergraph(p_pFilterGraph);
	raiseIfFailed(hr, "createBaseGraph::SetFiltergraph(p_pFilterGraph)");
}

void CBaseWinCaptureGraph::checkDevice()
{
	if (!p_pFileSink)
		return;

	//First Iteration
	TCHAR tempFileName[MAX_PATH + 2] = { 0 };
	_ttmpnam_s(tempFileName, MAX_PATH);

	HRESULT hr = p_pFileSink->SetFileName(tempFileName, nullptr);
	raiseIfFailed(hr, "checkDevice::SetFileName");

	hr = p_pMediaControl->Run();
	if (FAILED(hr))
	{
		DeleteFile(tempFileName);
		if (hr == HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES)) {
			throw DeviceIsBusyError();
		}
		else {
			throw MediaControlError("checkDevice::Run", hr);
		}
	}
	p_pMediaControl->Stop();
	DeleteFile(tempFileName);
}

void CBaseWinCaptureGraph::process(const TRecordInfo& info)
{
	HRESULT hr = p_pMediaControl->Run();
	if (FAILED(hr))
	{
		DeleteFile(info.FileName.c_str());
		if (hr == HRESULT_FROM_WIN32(ERROR_NO_SYSTEM_RESOURCES)) {
			throw DeviceIsBusyError("Process::Run::Device is busy");
		}
		else {
			throw MediaControlError("Process::Run::Failed", hr);
		}
	}

	p_bCapturing = true;
	long lCode = 0;
	p_pMediaEvent->WaitForCompletion(p_settings.DurationSec * 1000/*ms*/, &lCode);
	p_pMediaControl->Stop();
}

void CBaseWinCaptureGraph::stop()
{
	p_bShouldWork = false;
	p_pMediaControl->Stop();
	while (p_bCapturing) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

void CBaseWinCaptureGraph::freeFilters() {
	SAFE_RELEASE(p_pFileSink);
	SAFE_RELEASE(p_pMediaEvent);
	SAFE_RELEASE(p_pMediaControl);
	SAFE_RELEASE(p_pAudioSrc);
	SAFE_RELEASE(p_pVideoSrc);
	SAFE_RELEASE(p_pFilterGraph);
	SAFE_RELEASE(p_pCaptureGraphBuilder);
	SAFE_RELEASE(p_pProfile);
}

void CBaseWinCaptureGraph::raiseIfFailed(HRESULT hr, std::string error_msg)
{
	if (FAILED(hr)) {
		throw GraphInitializationError(error_msg, hr);
	}
}

std::wostream& operator << (std::wostream& ws, const SYSTEMTIME& st)
{
	wchar_t szTime[64] = { 0 };
	swprintf(szTime, 64,
		L"%d-%02d-%02d-%02d-%02d-%02d-%03d",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds
	);
	ws << szTime;
	return ws;
}

std::wstring CBaseWinCaptureGraph::getRecordFileName()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	wchar_t szTempPath[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, szTempPath);

	std::wstring sExtension;
	switch (GetType())
	{
	case EMediaType::Audio:
		sExtension = WAV_EXTENSION;
		break;
	case EMediaType::Photo:
		sExtension = JPG_EXTENSION;
		break;
	case EMediaType::Video:
	case EMediaType::AudioVideo:
	case EMediaType::Online:
		sExtension = WMV_EXTENSION;
		break;
	}

	std::wstringstream wss;
	wss << szTempPath << st << sExtension;
	return wss.str();
}
