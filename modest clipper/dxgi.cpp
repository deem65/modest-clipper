#include "modest_clipper.h"

#include <iostream>
#include <iterator>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using namespace std;

bool init_dxgi()
{
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> dctx;

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

    if (FAILED(hrCreation))
    {
        cerr << "D3D11 device creation failed\n";
        return false;
    }

    cout << "D3D11 device creation succeeded\n";

    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hrInterface = device.As(&dxgiDevice); 
    // device is a ComPtr<ID3D11Device>, which internally stores an
    // ID3D11Device* pointing to the COM object's ID3D11Device interface.
    //
    // because dxgiDevice is a ComPtr<IDXGIDevice>, As() knows at compile time
    // that we are asking for the IDXGIDevice interface.
    //
    // As() uses device's internal ID3D11Device* to call QueryInterface()
    // on that same COM object.
    // 
    // QueryInterface() is a COM function that lets you ask an existing COM object:
    // “do you support this other interface? if yes, give me a pointer to it”
    // 
    // if the COM object supports IDXGIDevice, QueryInterface() does not create
    // or find a different COM object. it gets the IDXGIDevice interface from
    // the **exact same COM object** that device already refers to, returns an
    // IDXGIDevice* to that interface, and As() stores that pointer in dxgiDevice.
    // 
    // device and dxgiDevice both give access to the **same underlying COM object**,
    // but they access it through different COM interfaces.
    //
    // device internally holds an ID3D11Device*.
    // dxgiDevice internally holds an IDXGIDevice*.
    //
    // Those pointers may have different addresses, but both were obtained from
    // the same COM object and expose different sets of functions on that object.

    if (FAILED(hrInterface)) {
        cerr << "failed to get idxgi interface";
        return false;
    }

    return true;
}