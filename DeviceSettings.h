#pragma once

enum class ECaptureQuality : uint8_t {
	Low,
	Medium,
	High,
};

enum class EDeviceType : uint8_t {
	Audio,
	Video,
	NotSet
};

struct TDeviceInfo
{
	bool Enabled;
	std::wstring Name;
	EDeviceType Type;
	ECaptureQuality Quality;

	TDeviceInfo() noexcept
		: Enabled(false), Type(EDeviceType::NotSet), Quality(ECaptureQuality::Medium) {}

	void SetName(std::string aName) { Name = std::wstring(Name.cbegin(), Name.cend()); }
	std::string aName() { return std::string(Name.cbegin(), Name.cend()); }
	void Clear() {
		Name.clear();
		Type = EDeviceType::NotSet;
		Quality = ECaptureQuality::Medium;
		Enabled = false;
	}
};

using TDevices = std::vector<TDeviceInfo>;
