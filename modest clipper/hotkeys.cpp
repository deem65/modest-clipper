#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <dxgi1_2.h>
#include "modest_clipper.h"

using namespace std;

int run() {
    constexpr int clipId = 1;

    HWND window = nullptr; //temp

    if (!RegisterHotKey(window, clipId, MOD_CONTROL | MOD_SHIFT, VK_F7)) {
        return 1;
    }
    cout << "running\n";

    MSG message{};

    while (GetMessage(&message, nullptr, 0, 0) > 0) {
        if (message.message == WM_HOTKEY && message.wParam == clipId) {
            clip(window);
        }
    }
    UnregisterHotKey(nullptr, clipId);

    return 0;
}