#include "PreCompiled.h"
#include "DeviceEnumerator.h"
#include "Logging.h"

#include <algorithm>

CDeviceEnumerator::CDeviceEnumerator()
	: m_pDevEnum(nullptr), m_pEnum(nullptr), m_pMoniker(nullptr)
{
	updateDeviceList();
}

CDeviceEnumerator::~CDeviceEnumerator() {
	SAFE_RELEASE(m_pEnum);
	SAFE_RELEASE(m_pDevEnum);
	for (IMoniker* pMoniker : m_monikers)
		SAFE_RELEASE(pMoniker);
}

const TDevices& CDeviceEnumerator::GetDevices() {
	return m_devices;
}

HRESULT CDeviceEnumerator::BindDevice(EDeviceType DeviceType, const std::wstring& DeviceName, IBaseFilter ** ppFilter)
{
	for (size_t i = 0; i < m_devices.size(); ++i) {
		auto info = m_devices.at(i);
		if (info.Type == DeviceType && info.Name == DeviceName) {
			HRESULT hr = m_monikers[i]->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)ppFilter);
			if (FAILED(hr)) {
				LogError("Moniker failed binding to object", hr);
			}
			return hr;
		}
	}
	return S_OK;
}

void CDeviceEnumerator::updateDeviceList()
{
	for (const auto& deviceType : { EDeviceType::Audio, EDeviceType::Video })
	{
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&m_pDevEnum);
		if (FAILED(hr)) {
			LogError("Devices::CoCreateInstance::IID_ICreateDevEnum. Code:%d", hr);
			goto CLEAN_UP;
		}

		hr = m_pDevEnum->CreateClassEnumerator(DeviceTypeToIID(deviceType), &m_pEnum, 0);
		if (hr == S_FALSE) {
			LogMessage("Devices::No devices");
			goto CLEAN_UP;
		}
		else if (FAILED(hr)) {
			LogError("Devices::CreateClassEnumerator", hr);
			goto CLEAN_UP;
		}

		while (m_pEnum->Next(1, &m_pMoniker, nullptr) == S_OK) {
			IPropertyBag* pPropBag = nullptr;
			hr = m_pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pPropBag);
			if (FAILED(hr)) {
				SAFE_RELEASE(m_pMoniker);
				continue;
			}

			TDeviceInfo info;
			VARIANT var;
			VariantInit(&var);

			hr = pPropBag->Read(L"FriendlyName", &var, 0);
			info.Type = deviceType;
			info.Name = SUCCEEDED(hr)
				? var.bstrVal
				: (deviceType == EDeviceType::Audio
					? L"Unknown audio input"
					: L"Unknown video input");

			LogDebug("Devices::Found " + info.aName());
			m_devices.push_back(std::move(info));
			m_monikers.push_back(m_pMoniker);

			VariantClear(&var);
			SAFE_RELEASE(pPropBag);
		}
		SAFE_RELEASE(m_pEnum);

		continue;
	}
CLEAN_UP:
	SAFE_RELEASE(m_pEnum);
	SAFE_RELEASE(m_pDevEnum);
}

IID DeviceTypeToIID(EDeviceType Type)
{
	switch (Type)
	{
	case EDeviceType::Audio:
		return CLSID_AudioInputDeviceCategory;
	case EDeviceType::Video:
		return CLSID_VideoInputDeviceCategory;
	case EDeviceType::NotSet:
		break;
	}
	throw std::runtime_error("Unknown DeviceType");
}
