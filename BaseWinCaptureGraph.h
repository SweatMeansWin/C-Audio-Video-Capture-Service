#pragma once

#include "BaseCaptureGraph.h"
#include "WinHeaders.h"
#include "WinRecordInfo.h"

#include <thread>


class CBaseWinCaptureGraph : public CBaseCaptureGraph {
public:
	CBaseWinCaptureGraph();
	virtual ~CBaseWinCaptureGraph() override;

	bool Create();
	void Start();
	void Stop();

protected:
	std::atomic_bool p_bShouldWork;
	std::atomic_bool p_bCapturing;

	IFilterGraph2*			p_pFilterGraph;
	ICaptureGraphBuilder2*	p_pCaptureGraphBuilder;
	IBaseFilter*			p_pVideoSrc;
	IBaseFilter*			p_pAudioSrc;
	IMediaEventEx*			p_pMediaEvent;
	IMediaControl*			p_pMediaControl;
	IFileSinkFilter*		p_pFileSink;
	IWMProfile*				p_pProfile;


	void createBaseGraph();
	void checkDevice();
	void stop();
	void raiseIfFailed(HRESULT hr, std::string error_msg);

	std::wstring getRecordFileName();
	
	virtual void process(const TRecordInfo& info);
	virtual void initFilters() = 0;
	virtual void freeFilters();
};
