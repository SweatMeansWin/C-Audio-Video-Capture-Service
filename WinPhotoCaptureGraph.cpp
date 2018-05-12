#include "PreCompiled.h"
#include "WinPhotoCaptureGraph.h"

#include "WinRecordInfo.h"
#include "Logging.h"
#include "DeviceEnumerator.h"
#include "Bitmap.h"

CWinPhotoCaptureGraph::CWinPhotoCaptureGraph() 
	: m_pSampleGrabber(nullptr), m_pSampleGrabberFilter(nullptr), m_pNullRenderer(nullptr) {
}

void CWinPhotoCaptureGraph::initFilters()
{
	createBaseGraph();
	CDeviceEnumerator devEnumerator;
	HRESULT hr = devEnumerator.BindDevice(EDeviceType::Video, p_settings.videoSettings.Name, &p_pVideoSrc);
	if (!p_pVideoSrc) {
		throw NoDeviceFound("initFilters::No video device found");
	}
	raiseIfFailed(hr, "initFilters::Bind to object failed");

	hr = p_pFilterGraph->AddFilter(p_pVideoSrc, nullptr);
	raiseIfFailed(hr, "initFilters::AddFilter(Video)");

	hr = CoCreateInstance(
		CLSID_SampleGrabber,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pSampleGrabberFilter));
	raiseIfFailed(hr, "initFilters::CoCreateInstance::CLSID_SampleGrabber");

	hr = p_pFilterGraph->AddFilter(m_pSampleGrabberFilter, NULL);
	raiseIfFailed(hr, "initFilters::CoCreateInstance::AddFilter(m_pSampleGrabberFilter)");

	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_ISampleGrabber");

	AM_MEDIA_TYPE mt = { 0 };
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = m_pSampleGrabber->SetMediaType(&mt);
	if (FAILED(hr)) {
		LogError("initFilters::SetMediaType(Video, RGB24)", hr);
	}

	hr = CoCreateInstance(
		CLSID_NullRenderer,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pNullRenderer));
	raiseIfFailed(hr, "initFilters::CoCreateInstance::CLSID_NullRenderer");

	hr = p_pFilterGraph->AddFilter(m_pNullRenderer, NULL);
	raiseIfFailed(hr, "initFilters::AddFilter(m_pNullRenderer)");

	hr = p_pCaptureGraphBuilder->RenderStream(
		NULL,						// Pin category.
		&MEDIATYPE_Video,			// Media type.
		p_pVideoSrc,
		NULL,
		m_pSampleGrabberFilter);
	raiseIfFailed(hr, "initFilters::RenderStream(Video,m_pSampleGrabberFilter)");

	hr = p_pCaptureGraphBuilder->RenderStream(
		NULL,						// Pin category.
		&MEDIATYPE_Video,			// Media type.
		m_pSampleGrabber,
		NULL,
		m_pNullRenderer);
	raiseIfFailed(hr, "initFilters::RenderStream(pSampleGrabber,m_pNullRenderer)");

	hr = p_pFilterGraph->QueryInterface(IID_IMediaEventEx, (void**)&p_pMediaEvent);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_IMediaEventEx");

	hr = p_pFilterGraph->QueryInterface(IID_IMediaControl, (void**)&p_pMediaControl);
	raiseIfFailed(hr, "initFilters::QueryInterface::IID_IMediaControl");

	hr = m_pSampleGrabber->SetOneShot(TRUE);
	if (FAILED(hr)) {
		LogError("initFilters::SetOneShot", (DWORD)hr);
	}
	hr = m_pSampleGrabber->SetBufferSamples(TRUE);
	if (FAILED(hr)) {
		LogError("initFilters::SetBufferSamples", (DWORD)hr);
	}
}

void CWinPhotoCaptureGraph::freeFilters()
{
	SAFE_RELEASE(m_pSampleGrabber);
	SAFE_RELEASE(m_pNullRenderer);
	SAFE_RELEASE(m_pSampleGrabberFilter);
	CBaseWinCaptureGraph::freeFilters();
}

void CWinPhotoCaptureGraph::process(const TRecordInfo& info)
{
	// Wait for one second
	CBaseWinCaptureGraph::process(info);
	// Process capture result, get image data
	// Find the required buffer size.
	long cbBuffer = 0;
	HRESULT hr = m_pSampleGrabber->GetCurrentBuffer(&cbBuffer, NULL);
	raiseIfFailed(hr, "Process::GetCurrentBuffer(size)");

	LPBYTE pBuffer = new BYTE[cbBuffer];
	hr = m_pSampleGrabber->GetCurrentBuffer(&cbBuffer, (long*)pBuffer);
	if (FAILED(hr)) {
		LogError("Process::GetCurrentBuffer()", hr);
		delete[] pBuffer;
		return;
	}

	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr)) {
		LogError("Process::GetConnectedMediaType()", hr);
		delete[] pBuffer;
		return;
	}

	if (mt.formattype == FORMAT_VideoInfo 
		&& mt.cbFormat >= sizeof(VIDEOINFOHEADER) 
		&& mt.pbFormat != nullptr)
	{
		VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);

		bool written = WriteBitmap(
			info.FileName,
			&pVih->bmiHeader,
			mt.cbFormat - SIZE_PREHEADER,
			pBuffer,
			cbBuffer
		);
		if (written) {
			CImage img;
			hr = img.Load(info.FileName.c_str());
			hr = img.Save(info.FileName.c_str(), Gdiplus::ImageFormatJPEG);
		}
	}
	if (mt.pbFormat) {
		CoTaskMemFree(mt.pbFormat);
		SAFE_RELEASE(mt.pUnk);
	}
	delete[] pBuffer;
}
