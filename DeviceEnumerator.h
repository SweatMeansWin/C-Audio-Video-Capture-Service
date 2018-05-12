#pragma once

#include "WinHeaders.h"
#include "DeviceSettings.h"


IID DeviceTypeToIID(EDeviceType Type);

class CDeviceEnumerator {
public:
	CDeviceEnumerator();
	~CDeviceEnumerator();

	const TDevices& GetDevices();
	HRESULT BindDevice(EDeviceType DeviceType, const std::wstring& DeviceName, IBaseFilter** ppFilter);

private:
	ICreateDevEnum * m_pDevEnum;
	IEnumMoniker* m_pEnum;
	IMoniker* m_pMoniker;

	TDevices m_devices;
	std::vector<IMoniker*> m_monikers;

	void updateDeviceList();
};
