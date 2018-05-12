#pragma once
#include "WinAudioVideoCaptureGraph.h"

#define DEFAULT_PORT 3038

class CWinOnlineCaptureGraph : public CWinAudioVideoCaptureGraph
{
public:
	CWinOnlineCaptureGraph();
	~CWinOnlineCaptureGraph();

	EMediaType GetType();
	std::string GetStreamUri();

protected:
	virtual void initFilters() override;
	virtual void freeFilters() override;
	
private:
	IWMWriterNetworkSink * m_pNetworkSink;

	std::string	m_streamUri;

	void addNetworkSink(DWORD port= DEFAULT_PORT);
	void removeNetworkSink();
};

