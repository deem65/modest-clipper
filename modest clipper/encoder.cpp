#include "modest_clipper.h"

#include <mfapi.h>
#include <mferror.h>
#include <iostream>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")

struct EncoderList {
	IMFActivate** items{};
	UINT32 count{};

	EncoderList() = default;
	EncoderList(const EncoderList&) = delete;
	EncoderList& operator=(const EncoderList&) = delete;

	~EncoderList()
	{
		for (UINT32 i = 0; i < count; ++i)
			items[i]->Release();

		CoTaskMemFree(items);
	}
};
bool report_failure(const char* operation, HRESULT hr) {
	std::cerr << operation << " failed: 0x"
		<< std::hex << static_cast<unsigned long>(hr)
		<< std::dec << std::endl;
	return false;
}
bool Encoder::init() {
	shutdown();

	if (!init_com() ||
		!init_mf() ||
		!create_hardware_encoder())
	{
		shutdown();
		return false;
	}

	std::cout << "h.264 activated\n";
	return true;
}
bool Encoder::init_com() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return report_failure("init com", hr);

	comStarted = true;
	return true;
}
bool Encoder::init_mf() {
	HRESULT hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
		return report_failure("init mf", hr);

	mfStarted = true;
	return true;
}
bool Encoder::create_hardware_encoder() {
	EncoderList availableEncoders;
	MFT_REGISTER_TYPE_INFO outputType{
		MFMediaType_Video,
		MFVideoFormat_H264
	};

	HRESULT hr = MFTEnumEx(
		MFT_CATEGORY_VIDEO_ENCODER,
		MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SORTANDFILTER,
		nullptr,
		&outputType,
		&availableEncoders.items,
		&availableEncoders.count
	);

	if (FAILED(hr))
		return report_failure("MFTEnumEx", hr);

	if (availableEncoders.count == 0) {
		std::cerr << "h.264 encoder not supported\n";
		return false;
	}

	for (UINT32 i = 0; i < availableEncoders.count; ++i) {
		hr = availableEncoders.items[i]->ActivateObject
		(
			IID_PPV_ARGS(transform.ReleaseAndGetAddressOf())
		);

		if (FAILED(hr)) {
			report_failure("ActivateObject", hr);
			availableEncoders.items[i]->ShutdownObject();
			transform.Reset();
			continue;
		}

		activation = availableEncoders.items[i];
		return true;
	}

	return false;
}
void Encoder::shutdown() noexcept {
	if (activation)
		activation->ShutdownObject();

	transform.Reset();
	activation.Reset();

	if (mfStarted) {
		MFShutdown();
		mfStarted = false;
	}

	if (comStarted) {
		CoUninitialize();
		comStarted = false;
	}
}
Encoder::~Encoder() { shutdown(); }
