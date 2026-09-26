#include <Windows.h>


bool get_window_dc(HDC& windowDc, HWND window) {
    windowDc = GetDC(window);
    return windowDc != nullptr;
}
bool get_memory_dc(HDC& memoryDc, HDC windowDc) {
    memoryDc = CreateCompatibleDC(windowDc);
    return memoryDc != nullptr;
}
bool get_window_dimensions(HWND window, int& width, int& height) {
    if (window == nullptr) {
        //multi monitor selection will be added later
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
        return true;
    }
    RECT rect{};
    if (!GetClientRect(window, &rect)) {
        return false;
    }
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    return width > 0 && height > 0;
}
bool get_window_bitmap(HBITMAP& windowBitmap, HDC windowDc, int width, int height) {
    windowBitmap = CreateCompatibleBitmap(windowDc, width, height);
    return windowBitmap != nullptr;
}
