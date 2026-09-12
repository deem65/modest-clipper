#include <atomic>
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "modest_clipper.h"
using namespace std;

atomic<bool> captureInProgress{ false };

void clip(HWND window) {
    bool prev = captureInProgress.exchange(true);

    if (prev) {
        return;
    }
    thread capture([window]() {
        capture_multi_frames(window);
        captureInProgress.store(false);
        });
    capture.detach();
}
void capture_multi_frames(HWND window)
{
    constexpr int fps = 5;
    constexpr int duration = 2;//s
    constexpr int frameCount = fps * duration;
    constexpr int intervalNs = 1'000'000'000 / fps;
    constexpr int bitsPerPixel = 32;

    auto interval = chrono::nanoseconds(intervalNs);
    auto nextTimePoint = chrono::steady_clock::now();

    FrameContext ctx{};
    if (!ctx.initialize(window)) {
        cerr << "frame context initialization: fail\n";
        return;
    }
    vector<Frame> frames;
    optional<Frame> frame;
    frames.reserve(frameCount);
    for (int i = 0; i < frameCount; i++) {
        frame = capture_frame(ctx, bitsPerPixel);

        if (frame.has_value()) {
            frames.push_back(move(frame.value()));
        }
        else {
            cerr << "frame" << i << " failed :(";
            //continues anyway
        }

        if (i < frameCount - 1) {
            nextTimePoint += interval;
            this_thread::sleep_until(nextTimePoint);
        }
    }
    for (int i = 0; i < frames.size(); i++) {
        save_bitmap(frames[i].bitmapHeader, frames[i].pixelBytes, i);
    }

    cout << "requested frames: " << frameCount << '\n';
    cout << "captured frames: " << frames.size() << '\n';
    cout << "failed frames: " << frameCount - frames.size() << '\n';
}

optional<Frame> capture_frame(FrameContext& ctx, int bits)
{
    HGDIOBJ previousSelectedObject = SelectObject(ctx.memoryDc, ctx.windowBitmap);

    if (previousSelectedObject == nullptr) {
        return nullopt;
    }
    if (!BitBlt(ctx.memoryDc, 0, 0, ctx.width, ctx.height, ctx.windowDc, 0, 0, SRCCOPY)) {
        SelectObject(ctx.memoryDc, previousSelectedObject);
        return nullopt;
    }
    SelectObject(ctx.memoryDc, previousSelectedObject); //GetDIBits expects the bitmap to be unselected in a dc

    return extract_frame_from_bitmap(ctx.windowDc, ctx.windowBitmap, ctx.width, ctx.height, bits);

}
bool FrameContext::initialize(HWND targetWindow)
{
    window = targetWindow;

    if (
        !get_window_dc(windowDc, window) ||
        !get_memory_dc(memoryDc, windowDc) ||
        !get_window_dimensions(window, width, height) ||
        !get_window_bitmap(windowBitmap, windowDc, width, height)
        )
    {
        cleanup();
        return false;
    }

    return true;
}
void FrameContext::cleanup()
{
    if (windowBitmap != nullptr) {
        DeleteObject(windowBitmap);
        windowBitmap = nullptr;
    }
    if (memoryDc != nullptr) {
        DeleteDC(memoryDc);
        memoryDc = nullptr;
    }
    if (windowDc != nullptr) {
        ReleaseDC(window, windowDc);
        windowDc = nullptr;
    }
}

FrameContext::~FrameContext()
{
    cleanup();
}
