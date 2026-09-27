#include "modest_clipper.h"
#include <iostream>

bool inspect_frame(const DxgiFrame& frame) {
	CD3D11_TEXTURE2D_DESC desc{};
	frame.texture->GetDesc(&desc);

	std::cout 
		<< "height: " << desc.Height << std::endl
		<< "width:" << desc.Width << std::endl
		<< "format:" << desc.Format << std::endl;

	return true;
}
void loop() {
	Dxgi dxgi{};
	DxgiFrame f{};

	if (!dxgi.init())
		return;

	while (true) {
		if (!dxgi.try_get_frame(f, 100)) {
			continue;
		}
		if (!inspect_frame(f)) {
			continue;
		}
	}
}