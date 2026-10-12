#include "modest_clipper.h"
#include <iostream>

void inspect_frame(const dxgi_frame& frame) {
	D3D11_TEXTURE2D_DESC desc{};
	frame.texture->GetDesc(&desc);

	std::cout 
		<< "height: " << desc.Height << std::endl
		<< "width:" << desc.Width << std::endl
		<< "format:" << desc.Format << std::endl
		<< std::endl;
}
void capture_loop()
{
    encoder encoder{};
    if (!encoder.init())
        return;
    dxgi dxgi{};
    if (!dxgi.init())
        return;
    if (!encoder.set_device(dxgi.get_device()))
        return;

    dxgi_frame f{};

    while (clip_running) {
        if (!dxgi.try_get_frame(f, 100))
            continue;

        inspect_frame(f);
    }
}