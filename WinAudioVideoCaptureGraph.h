#pragma once
#include "BaseWinCaptureGraph.h"

class CWinAudioVideoCaptureGraph : public CBaseWinCaptureGraph
{
public:
	CWinAudioVideoCaptureGraph();
	~CWinAudioVideoCaptureGraph() = default;

	EMediaType GetType();

protected:
	IBaseFilter * p_pASFWriter;
	IWMWriterAdvanced2*	p_pWMWA2;

	void loadProfile();
	void configureProfile();
	void setQuality(IWMPropertyVault* pProperty, ECaptureQuality Quality);

	virtual void initFilters() override;
	virtual void freeFilters() override;
	virtual void process(const TRecordInfo& info) override;
};

