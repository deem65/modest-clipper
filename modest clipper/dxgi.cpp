#include "modest_clipper.h"

#include <iostream>
#include <iterator>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using namespace std;

bool init_dxgi()
{
    ComPtr<ID3D11Device> d;
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
        d.GetAddressOf(),           //output >> where to write device
        &config.selectedFeatureLvl, //output >> which version of d3d feature level was selected out of config.featureLvls
        dctx.GetAddressOf());       //output >> where to write device context

    if (FAILED(hr))
    {
        cerr << "D3D11 device creation failed\n";
        return false;
    }

    cout << "D3D11 device creation succeeded\n";
    return true;
}