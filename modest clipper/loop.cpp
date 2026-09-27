#include "modest_clipper.h"
#include <iostream>

void inspect_frame(const DxgiFrame& frame) {
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
    Encoder encoder{};

    if (!encoder.init())
        return;

    Dxgi dxgi{};
    DxgiFrame f{};

    if (!dxgi.init())
        return;

    while (clip_running) {
        if (!dxgi.try_get_frame(f, 100))
            continue;

        inspect_frame(f);
    }
}