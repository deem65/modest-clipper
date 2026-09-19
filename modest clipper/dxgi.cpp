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

    HRESULT hr = D3D11CreateDevice(
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

    if (FAILED(hr))
    {
        cerr << "D3D11 device creation failed\n";
        return false;
    }

    cout << "D3D11 device creation succeeded\n";

    ComPtr<IDXGIDevice> dxgiDevice;
    device.As(&dxgiDevice); //does the com object device refer to implement dxgiDevice's 
                            //template type (known at compile time)? if it does, As() 
                            //asks for the object behind, device for IDXGIDevice, gets 
                            //back a ptr to that IDXGIDevice interface, and stores that
                            //ptr in dxgiDevice.

    return true;
}