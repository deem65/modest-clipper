#pragma once

#include <Windows.h>
#include <vector>
#include <optional>

struct Frame {
    BITMAPINFOHEADER bitmapHeader{};
    std::vector<unsigned char> pixelBytes;
};

class FrameContext
{
public:
    FrameContext() = default;
    FrameContext(const FrameContext&) = delete; //disable copy initialization
    FrameContext& operator=(const FrameContext&) = delete; //disable copy assignment
    HWND window{};
    HDC windowDc{};
    HDC memoryDc{};
    HBITMAP windowBitmap{};

    int width{};
    int height{};

    bool initialize(HWND targetWindow);
    void cleanup();

    ~FrameContext();
};

void clip(HWND window);

void capture_multi_frames(HWND window);

void save_bitmap(
    const BITMAPINFOHEADER& bitmapHeader,
    const std::vector<unsigned char>& pixelBytes,
    int frameNum
);
int run();
bool get_window_dc(HDC& windowDc, HWND window);
bool get_memory_dc(HDC& memoryDc, HDC screenDc);
bool get_window_dimensions(HWND window, int& width, int& height);
bool get_window_bitmap(HBITMAP& windowBitmap, HDC windowDc, int width, int height);

BITMAPINFO create_bitmap_info(int screenWidth, int screenHeight, int bitsPerPixel);

std::optional<Frame> capture_frame(FrameContext& context, int bits);

std::optional<Frame> extract_frame_from_bitmap(HDC screenDc, HBITMAP screenBitmap, int screenWidth, int screenHeight, int bitsPerPixel);



