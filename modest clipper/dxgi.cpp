#include "modest_clipper.h"

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using namespace std;

bool Dxgi::init()
{
    dcfg config{};

    HRESULT hrCreation = D3D11CreateDevice(
        config.adapter,
        config.driverType,
        config.software,
        config.flags,
        config.featureLvls.data(),
        static_cast<UINT>(config.featureLvls.size()),
        config.sdk,
        device.GetAddressOf(),      //output >> where to write device
        &config.selectedFeatureLvl, //output >> which version of d3d feature level was selected out of config.featureLvls
        dctx.GetAddressOf());       //output >> where to write device context

    if (!check_hr(hrCreation, "d3dd1 device creation failed"))
        return false;

    /*
    device is a ComPtr<ID3D11Device>, which internally stores an
    ID3D11Device* pointing to the COM object's ID3D11Device interface.
    because dxgiDevice is a ComPtr<IDXGIDevice>, As() knows at compile time
    that we are asking for the IDXGIDevice interface.

    As() uses device's internal ID3D11Device* to call QueryInterface()
    on that same COM object.

    QueryInterface() is a COM function that lets you ask an existing COM object:
    “do you support this other interface? if yes, give me a pointer to it”

    if the COM object supports IDXGIDevice, QueryInterface() does not create
    or find a different COM object. it gets the IDXGIDevice interface from
    the **exact same COM object** that device already refers to, returns an
    IDXGIDevice* to that interface, and As() stores that pointer in dxgiDevice.

    device and dxgiDevice both give access to the **same COM object**,
    but they access it through different COM interfaces.

    device internally holds an ID3D11Device*.
    dxgiDevice internally holds an IDXGIDevice*.

    Those pointers may have different addresses, but both were obtained from
    the same COM object and expose different sets of functions on that object.
    */

    return
            (
            check_hr(device.As(&dxgiDevice), "failed to get idxgi interface") &&
            check_hr(dxgiDevice->GetAdapter(adapter.GetAddressOf()), "failed to get idxgi adapter") &&
            check_hr(adapter->EnumOutputs(0, output.GetAddressOf()), "failed to get idxgi output") &&
            check_hr(output.As(&output1), "failed to get idxgi output1") &&
            check_hr(output1->DuplicateOutput(device.Get(), duplication.GetAddressOf()), "failed to create desktop duplication")
            );
}
bool Dxgi::try_get_frame(DxgiFrame& frame, UINT timeoutMs)
{
    frame.end();
    ComPtr<IDXGIResource> resource;
    if (!acquire_frame(frame, resource, timeoutMs))
        return false;
    if (!get_frame_texture(frame, resource))
    {
        frame.end();
        return false;
    }
    return true;
}
bool Dxgi::acquire_frame(DxgiFrame& frame, ComPtr<IDXGIResource>& resource, UINT timeoutMs)
{
    DXGI_OUTDUPL_FRAME_INFO info{};
    HRESULT hrFrame = duplication->AcquireNextFrame(timeoutMs, &info, resource.GetAddressOf());
    if (hrFrame == DXGI_ERROR_WAIT_TIMEOUT)
        return false;
    if (!check_hr(hrFrame, "failed to acquire frame"))
        return false;
    frame.info = info;
    frame.duplication = duplication;

    return true;
}
bool Dxgi::get_frame_texture(DxgiFrame& frame, ComPtr<IDXGIResource>& resource)
{
    return check_hr(
        resource.As(&frame.texture),
        "failed to get frame texture"
    );
}
bool check_hr(HRESULT hr, std::string_view m)
{
    if (SUCCEEDED(hr))
        return true;

    cerr << m << endl;
    return false;
}
