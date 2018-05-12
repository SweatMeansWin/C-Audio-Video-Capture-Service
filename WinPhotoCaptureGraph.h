#pragma once
#include "BaseWinCaptureGraph.h"

class CWinPhotoCaptureGraph :
	public CBaseWinCaptureGraph
{
public:
	CWinPhotoCaptureGraph();
	~CWinPhotoCaptureGraph() = default;

	EMediaType GetType() {
		return EMediaType::Photo;
	}

protected:
	ISampleGrabber * m_pSampleGrabber;
	IBaseFilter* m_pNullRenderer;
	IBaseFilter* m_pSampleGrabberFilter;
	
	virtual void initFilters() override;
	virtual void freeFilters() override;
	virtual void process(const TRecordInfo& info) override;
};
