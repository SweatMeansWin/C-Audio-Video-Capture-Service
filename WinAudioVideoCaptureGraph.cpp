#include "PreCompiled.h"
#include "WinAudioVideoCaptureGraph.h"
#include "WinRecordInfo.h"
#include "DeviceEnumerator.h"
#include "Logging.h"

#include <fstream>

LPCWSTR sAudioVideoProfileName = L"AudioVideo.prx";
LPCWSTR sVideoProfileName = L"Video.prx";
LPCWSTR sAudioProfileName = L"Audio.prx";

std::map<ECaptureQuality, BYTE> mp_qualityMapping = {
	{ECaptureQuality::Low, 25},
	{ECaptureQuality::Medium, 55},
	{ECaptureQuality::High, 80},
};

CWinAudioVideoCaptureGraph::CWinAudioVideoCaptureGraph() 
	: p_pASFWriter(nullptr), p_pWMWA2(nullptr) {
}

EMediaType CWinAudioVideoCaptureGraph::GetType()
{
	if (p_settings.videoSettings.Enabled) {
		if (p_settings.audioSettings.Enabled) {
			return EMediaType::AudioVideo;
		}
		return EMediaType::Video;
	}
	return EMediaType::Audio;
}

void CWinAudioVideoCaptureGraph::setQuality(IWMPropertyVault* pProperty, ECaptureQuality Quality)
{
	BYTE buffer = TRUE;
	HRESULT hr = pProperty->SetProperty(g_wszVBREnabled, WMT_TYPE_BOOL, &buffer, sizeof(BYTE));
	raiseIfFailed(hr, "SetProperty::g_wszVBREnabled");

	buffer = mp_qualityMapping.at(Quality);
	hr = pProperty->SetProperty(g_wszVBRQuality, WMT_TYPE_DWORD, &buffer, sizeof(BYTE));
	raiseIfFailed(hr, "SetProperty::g_wszVBRQuality");

	buffer = 0;
	hr = pProperty->SetProperty(g_wszVBRBitrateMax, WMT_TYPE_DWORD, &buffer, sizeof(BYTE));
	raiseIfFailed(hr, "SetProperty::g_wszVBRBitrateMax");
	hr = pProperty->SetProperty(g_wszVBRBufferWindowMax, WMT_TYPE_DWORD, &buffer, sizeof(BYTE));
	raiseIfFailed(hr, "SetProperty::g_wszVBRBufferWindowMax");
}

void CWinAudioVideoCaptureGraph::loadProfile()
{
	std::wstring sProfilePath = p_settings.systemSettings.ProfileDirectory;

	if (p_settings.videoSettings.Enabled) {
		if (p_settings.audioSettings.Enabled) {
			sProfilePath.append(sAudioVideoProfileName);
		}
		else {
			sProfilePath.append(sVideoProfileName);
		}
	}
	else {
		sProfilePath.append(sVideoProfileName);
	}

	std::wifstream wif(sProfilePath, std::ios::binary);
	if (!wif.is_open()) {
		std::stringstream ss;
		ss << "Unable to open profile stream [" << sProfilePath << "]: ";
		throw MediaControlError(ss.str(), errno);
	}

	IWMProfileManager *pProfileManager = nullptr;
	HRESULT hr = WMCreateProfileManager(&pProfileManager);
	raiseIfFailed(hr, "WMCreateProfileManager");
	if (pProfileManager)
	{
		wif.seekg(0, std::ios::end);
		std::vector<wchar_t> buffer(static_cast<const unsigned int>(wif.tellg()));
		wif.seekg(0, std::ios::beg);
		wif.read((wchar_t*)&buffer[0], buffer.size());
		wif.close();

		hr = pProfileManager->LoadProfileByData(buffer.data(), &p_pProfile);
		SAFE_RELEASE(pProfileManager);
		raiseIfFailed(hr, "LoadProfileByData");
	}
}

void CWinAudioVideoCaptureGraph::configureProfile()
{
	loadProfile();
	DWORD streamCount = 0;
	HRESULT hr = p_pProfile->GetStreamCount(&streamCount);
	if (SUCCEEDED(hr) && streamCount > 0) {
		IWMStreamConfig* pConfig = nullptr;
		for (DWORD streamIdx = 0; streamIdx < streamCount; ++streamIdx) {
			hr = p_pProfile->GetStream(streamIdx, &pConfig);
			if (SUCCEEDED(hr) && pConfig) {
				GUID streamGuid;
				pConfig->GetStreamType(&streamGuid);
				if (streamGuid == WMMEDIATYPE_Audio && p_settings.audioSettings.Enabled) {
					IWMPropertyVault* pProperty = nullptr;
					hr = pConfig->QueryInterface<IWMPropertyVault>(&pProperty);
					if (SUCCEEDED(hr) && pProperty)
					{
						setQuality(pProperty, p_settings.audioSettings.Quality);
						SAFE_RELEASE(pProperty);
						hr = p_pProfile->ReconfigStream(pConfig);
						if (FAILED(hr)) {
							LogError("ReconfigStream failed for audio", hr);
						}
					}
				}
				else if (streamGuid == WMMEDIATYPE_Video && p_settings.videoSettings.Enabled)
				{
					IWMPropertyVault* pProperty = nullptr;
					hr = pConfig->QueryInterface<IWMPropertyVault>(&pProperty);
					if (SUCCEEDED(hr) && pProperty) {
						setQuality(pProperty, p_settings.videoSettings.Quality);
						SAFE_RELEASE(pProperty);
						hr = p_pProfile->ReconfigStream(pConfig);
						if (FAILED(hr))
							LogError("ReconfigStream failed for video", hr);
					}
				}
				SAFE_RELEASE(pConfig);
			}
		}
	}
}

void CWinAudioVideoCaptureGraph::process(const TRecordInfo& info)
{
	HRESULT hr = p_pFileSink->SetFileName(info.FileName.c_str(), nullptr);
	raiseIfFailed(hr, "Process::SetFileName");
	CBaseWinCaptureGraph::process(info);
}

void CWinAudioVideoCaptureGraph::initFilters()
{
	createBaseGraph();
	//configureProfile();
	HRESULT hr = 0;
	CDeviceEnumerator enumerator;

	//GET ASF WRITER
	hr = CoCreateInstance(CLSID_WMAsfWriter, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&p_pASFWriter);
	raiseIfFailed(hr, "initFilters::CoCreateInstance::CLSID_WMAsfWriter");

	hr = p_pFilterGraph->AddFilter(p_pASFWriter, nullptr);
	raiseIfFailed(hr, "initFilters::AddFilter(p_pASFWriter)");

	//CONFIGURE ASF WRITER
	IConfigAsfWriter *pWriterConfig = nullptr;
	hr = p_pASFWriter->QueryInterface(IID_IConfigAsfWriter, (void**)&pWriterConfig);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_IConfigAsfWriter");

	if (p_pProfile != nullptr) {
		hr = pWriterConfig->ConfigureFilterUsingProfile(p_pProfile);
	}
	if (FAILED(hr) || p_pProfile == nullptr)
	{
		IWMProfileManager *pProfileManager = nullptr;
		hr = WMCreateProfileManager(&pProfileManager);
		raiseIfFailed(hr, "initFilters::WMCreateProfileManager");

		if (pProfileManager)
		{
			IWMProfile *pDefProfile = nullptr;
			hr = pProfileManager->LoadProfileByID(WMProfile_V80_56VideoOnly, &pDefProfile);
			SAFE_RELEASE(pProfileManager);
			raiseIfFailed(hr, "initFilters::LoadProfileByID");

			hr = pWriterConfig->ConfigureFilterUsingProfile(pDefProfile);
			SAFE_RELEASE(pWriterConfig);
			SAFE_RELEASE(pDefProfile);
			raiseIfFailed(hr, "initFilters::ConfigureFilterUsingProfile");
		}
	}
	SAFE_RELEASE(pWriterConfig);

	IServiceProvider *pProvider = nullptr;
	hr = p_pASFWriter->QueryInterface(IID_IServiceProvider, (void**)&pProvider);
	if (SUCCEEDED(hr))
	{
		hr = pProvider->QueryService(
			IID_IWMWriterAdvanced2,
			IID_IWMWriterAdvanced2,
			(void**)&p_pWMWA2);
		SAFE_RELEASE(pProvider);
		raiseIfFailed(hr, "initFilters::QueryService");
	}

	if (p_pWMWA2) {
		hr = p_pWMWA2->SetLiveSource(TRUE);
		raiseIfFailed(hr, "initFilters::SetLiveSource");
	}

	if (p_settings.videoSettings.Enabled)
	{
		hr = enumerator.BindDevice(EDeviceType::Video, p_settings.videoSettings.Name, &p_pVideoSrc);
		// S_OK and empty filter means we cannot find
		if (!p_pVideoSrc) {
			throw NoDeviceFound("initFilters::No video device found");
		}
		raiseIfFailed(hr, "initFilters::Bind to object failed");

		hr = p_pFilterGraph->AddFilter(p_pVideoSrc, nullptr);
		raiseIfFailed(hr, "initFilters::AddFilter(Video)");
		
		hr = p_pCaptureGraphBuilder->RenderStream(
			&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video,
			p_pVideoSrc,
			nullptr,
			p_pASFWriter
		);
		raiseIfFailed(hr, "initFilters::RenderStream(Video,ASF)");
	}

	if (p_settings.audioSettings.Enabled)
	{
		hr = enumerator.BindDevice(EDeviceType::Audio, p_settings.audioSettings.Name, &p_pAudioSrc);
		// S_OK and empty filter means we cannot find
		if (!p_pAudioSrc) {
			throw NoDeviceFound("initFilters::No audio device found");
		}
		raiseIfFailed(hr, "initFilters::Bind to object failed");

		hr = p_pFilterGraph->AddFilter(p_pAudioSrc, nullptr);
		raiseIfFailed(hr, "initFilters::AddFilter(Audio)");

		hr = p_pCaptureGraphBuilder->RenderStream(
			&PIN_CATEGORY_CAPTURE,		// Pin category.
			&MEDIATYPE_Audio,			// Media type.
			p_pAudioSrc,
			nullptr,					// Intermediate filter (optional).
			p_pASFWriter
		);
		raiseIfFailed(hr, "initFilters::RenderStream(Audio,ASF)");
	}

	hr = p_pASFWriter->QueryInterface(IID_IFileSinkFilter, (void**)&p_pFileSink);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_IFileSinkFilter");

	hr = p_pFilterGraph->QueryInterface(IID_IMediaEvent, (void**)&p_pMediaEvent);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_IMediaEvent");

	hr = p_pFilterGraph->QueryInterface(IID_IMediaControl, (void**)&p_pMediaControl);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_IMediaControl");
}

void CWinAudioVideoCaptureGraph::freeFilters()
{
	SAFE_RELEASE(p_pASFWriter);
	SAFE_RELEASE(p_pWMWA2);
	CBaseWinCaptureGraph::freeFilters();
}
