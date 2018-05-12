#include "PreCompiled.h"
#include "WinOnlineCaptureGraph.h"

#include "Logging.h"

CWinOnlineCaptureGraph::CWinOnlineCaptureGraph() : m_pNetworkSink(nullptr)
{
}


CWinOnlineCaptureGraph::~CWinOnlineCaptureGraph() {
}

EMediaType CWinOnlineCaptureGraph::GetType() {
	return EMediaType::Online;
}

std::string CWinOnlineCaptureGraph::GetStreamUri() {
	return m_streamUri;
}

void CWinOnlineCaptureGraph::initFilters() {
	CWinAudioVideoCaptureGraph::initFilters();
	addNetworkSink();
}

void CWinOnlineCaptureGraph::freeFilters() {
	SAFE_RELEASE(m_pNetworkSink);
	CWinAudioVideoCaptureGraph::freeFilters();
}

void CWinOnlineCaptureGraph::addNetworkSink(DWORD port) {
	// Clear previous
	SAFE_RELEASE(m_pNetworkSink);
	m_streamUri.clear();

	HRESULT hr = WMCreateWriterNetworkSink(&m_pNetworkSink);
	raiseIfFailed(hr, "AddNetworkSink::WMCreateWriterNetworkSink");

	hr = m_pNetworkSink->Open(&port);
	raiseIfFailed(hr, "addNetworkSink::OpenPort");

	WCHAR urlBuffer[MAX_PATH] = { 0 };
	DWORD urlBufferSize = MAX_PATH;

	hr = m_pNetworkSink->GetHostURL(urlBuffer, &urlBufferSize);
	raiseIfFailed(hr, "AddNetworkSink::GetHostURL");

	hr = p_pWMWA2->AddSink(m_pNetworkSink);
	raiseIfFailed(hr, "AddNetworkSink::AddSink");

	std::wstring ws(urlBuffer);	
	std::stringstream ss;
	ss << ws << "/AVStreaming.asf";
	m_streamUri = std::move(ss.str());
}

void CWinOnlineCaptureGraph::removeNetworkSink() {
	if (m_pNetworkSink && p_pWMWA2) {
		m_pNetworkSink->Disconnect();
		m_pNetworkSink->Close();
		p_pWMWA2->RemoveSink(nullptr);
		m_streamUri.clear();
	}
}
