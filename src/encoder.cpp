#include "modest_clipper.h"

#include <mfapi.h>
#include <mferror.h>
#include <iostream>
#include <d3d10.h>
#include <codecapi.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")

struct encoder_list {
	IMFActivate** items{};
	UINT32 count{};

	encoder_list() = default;
	encoder_list(const encoder_list&) = delete;
	encoder_list& operator=(const encoder_list&) = delete;
  
	~encoder_list()
	{
		for (int i = 0; i < count; ++i)
			items[i]->Release();

		CoTaskMemFree(items);  
	}
};

static bool report_failure(const char* operation, const HRESULT hr) {
	std::cerr << operation << " failed: 0x"
		<< std::hex << static_cast<unsigned long>(hr)
		<< std::dec << '\n';
	return false;
}
bool encoder::init() {
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
bool encoder::init_com() {
	if (const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED); FAILED(hr))
		return report_failure("init com", hr);

	comStarted = true;
	return true;
}
bool encoder::init_mf() {
	if (const HRESULT hr = MFStartup(MF_VERSION); FAILED(hr))
		return report_failure("init mf", hr);

	mfStarted = true;
	return true;
}
bool encoder::create_hardware_encoder() {
	encoder_list availableEncoders;
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
bool encoder::set_device(ID3D11Device* device)
{
	if (!device || !transform)
		return report_failure("set_device: missing device or encoder", E_POINTER);

	Microsoft::WRL::ComPtr<IMFAttributes> attributes;

	HRESULT hr = transform->GetAttributes(attributes.GetAddressOf());
	if (FAILED(hr))
		return report_failure("GetAttributes", hr);

	hr = attributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
	if (FAILED(hr))
		return report_failure("unlock asynchronous encoder", hr);

	UINT32 d3d11_aware{};
	hr = attributes->GetUINT32(MF_SA_D3D11_AWARE, &d3d11_aware);
	if (FAILED(hr))
		return report_failure("query D3D11 support", hr);

	if (!d3d11_aware) {
		std::cerr << "selected encoder does not support D3D11 resources.\n";
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D10Multithread> multithread;

	hr = device->QueryInterface(
		IID_PPV_ARGS(multithread.GetAddressOf())
	);
	if (FAILED(hr))
		return report_failure("query multithread protection", hr);

	multithread->SetMultithreadProtected(TRUE);

	Microsoft::WRL::ComPtr<IMFDXGIDeviceManager> manager;
	UINT reset_token{};

	hr = MFCreateDXGIDeviceManager(
		&reset_token,
		manager.GetAddressOf()
	);
	if (FAILED(hr))
		return report_failure("create device manager", hr);

	hr = manager->ResetDevice(device, reset_token);
	if (FAILED(hr))
		return report_failure("associate D3D11 device", hr);

	hr = transform->ProcessMessage(
		MFT_MESSAGE_SET_D3D_MANAGER,
		reinterpret_cast<ULONG_PTR>(manager.Get())
	);
	if (FAILED(hr))
		return report_failure("connect device manager to encoder", hr);

	deviceManager = std::move(manager);

	std::cout << "encoder accepted the d3d11 device manager.\n";
	return true;
}

static HRESULT set_video_format( IMFMediaType* type, const GUID& subtype, UINT width, UINT height, UINT fps)
{
	HRESULT hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	if (FAILED(hr))
		return hr;

	hr = type->SetGUID(MF_MT_SUBTYPE, subtype);
	if (FAILED(hr))
		return hr;

	hr = MFSetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
	if (FAILED(hr))
		return hr;

	hr = MFSetAttributeRatio(type, MF_MT_FRAME_RATE, fps, 1);
	if (FAILED(hr))
		return hr;

	hr = MFSetAttributeRatio(type, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	if (FAILED(hr))
		return hr;

	return type->SetUINT32(
		MF_MT_INTERLACE_MODE,
		MFVideoInterlace_Progressive
	);
}
bool encoder::configure(UINT width, UINT height, UINT fps, UINT bitrate)
{
	if (!transform || !deviceManager)
		return report_failure("configure: encoder/device not ready", E_UNEXPECTED);

	if (!width || !height || !fps || !bitrate ||
		width % 2 != 0 || height % 2 != 0)
	{
		return report_failure("configure: invalid video settings", E_INVALIDARG);
	}

	DWORD input_count{};
	DWORD output_count{};

	HRESULT hr = transform->GetStreamCount(&input_count, &output_count);
	if (FAILED(hr))
		return report_failure("GetStreamCount", hr);

	if (input_count != 1 || output_count != 1)
		return report_failure("expected one input and one output", E_UNEXPECTED);

	hr = transform->GetStreamIDs(
		1, &inputStreamId,
		1, &outputStreamId
	);

	if (hr == E_NOTIMPL) {
		inputStreamId = 0;
		outputStreamId = 0;
	}
	else if (FAILED(hr)) {
		return report_failure("GetStreamIDs", hr);
	}

	Microsoft::WRL::ComPtr<IMFMediaType> output_type;

	hr = MFCreateMediaType(output_type.GetAddressOf());
	if (FAILED(hr))
		return report_failure("create output type", hr);

	hr = set_video_format(
		output_type.Get(), MFVideoFormat_H264, width, height, fps
	);
	if (FAILED(hr))
		return report_failure("set output attributes", hr);

	hr = output_type->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
	if (FAILED(hr))
		return report_failure("set bitrate", hr);

	hr = output_type->SetUINT32(
		MF_MT_MPEG2_PROFILE,
		eAVEncH264VProfile_Main
	);
	if (FAILED(hr))
		return report_failure("set H.264 profile", hr);

	hr = transform->SetOutputType(outputStreamId, output_type.Get(), 0);
	if (FAILED(hr))
		return report_failure("SetOutputType H.264", hr);

	Microsoft::WRL::ComPtr<IMFMediaType> input_type;

	hr = MFCreateMediaType(input_type.GetAddressOf());
	if (FAILED(hr))
		return report_failure("create input type", hr);

	hr = set_video_format(
		input_type.Get(), MFVideoFormat_NV12, width, height, fps
	);
	if (FAILED(hr))
		return report_failure("set input attributes", hr);

	hr = transform->SetInputType(inputStreamId, input_type.Get(), 0);
	if (FAILED(hr))
		return report_failure("SetInputType NV12", hr);

	std::cout << "Encoder configured: "
		<< width << 'x' << height
		<< " at " << fps << " FPS, "
		<< bitrate << " bits/sec\n";

	return true;
}
void encoder::shutdown() noexcept {
	if (activation)
		activation->ShutdownObject();

	transform.Reset();
	activation.Reset();
	deviceManager.Reset();

	if (mfStarted) {
		MFShutdown();
		mfStarted = false;
	}

	if (comStarted) {
		CoUninitialize();
		comStarted = false;
	}
}
encoder::~encoder() { shutdown(); }
