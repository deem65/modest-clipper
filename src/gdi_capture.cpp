#include <atomic>
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "modest_clipper.h"

using namespace std;

namespace
{
    atomic capture_in_progress{false};
}  

void clip(HWND window) {
    if (const bool prev = capture_in_progress.exchange(true)) {
        return;
    }
    thread capture([window] {
        capture_multi_frames(window);
        capture_in_progress.store(false);
        });
    capture.detach();
}
void capture_multi_frames(const HWND window)
{
    constexpr int fps = 5;
    constexpr int duration = 2; // s
    constexpr int frame_count = fps * duration;
    constexpr int interval_ns = 1'000'000'000 / fps;

    constexpr auto interval = chrono::nanoseconds(interval_ns);
    auto next_time_point = chrono::steady_clock::now();

    FrameContext ctx{};
    if (!ctx.initialize(window)) {
        cerr << "frame context initialization: fail\n";
        return;
    }
    vector<Frame> frames;
    frames.reserve(frame_count);
    for (int i = 0; i < frame_count; i++) {
        constexpr int bits_per_pixel = 32;

        if (optional<Frame> frame = capture_frame(ctx, bits_per_pixel); frame.has_value()) {
            frames.push_back(std::move(frame.value()));
        }
        else {
            cerr << "frame" << i << " failed :(";
            // continues anyway
        }

        if (i < frame_count - 1) {
            next_time_point += interval;
            this_thread::sleep_until(next_time_point);
        }
    }
    for (size_t i = 0; i < frames.size(); i++) {
        save_bitmap(frames[i].bitmapHeader, frames[i].pixelBytes, static_cast<int>(i));
    }

    cout << "requested frames: " << frame_count << '\n';
    cout << "captured frames: " << frames.size() << '\n';
    cout << "failed frames: " << frame_count - frames.size() << '\n';
}

optional<Frame> capture_frame(FrameContext& ctx, const int bits)
{
    const HGDIOBJ previous_selected_object = SelectObject(ctx.memoryDc, ctx.windowBitmap);

    if (previous_selected_object == nullptr) {
        return nullopt;
    }
    if (!BitBlt(ctx.memoryDc, 0, 0, ctx.width, ctx.height, ctx.windowDc, 0, 0, SRCCOPY)) {
        SelectObject(ctx.memoryDc, previous_selected_object);
        return nullopt;
    }
    SelectObject(ctx.memoryDc, previous_selected_object); // GetDIBits expects the bitmap to be unselected in a dc

    return extract_frame_from_bitmap(ctx.windowDc, ctx.windowBitmap, ctx.width, ctx.height, bits);

}
bool FrameContext::initialize(const HWND target_window)
{
    window = target_window;

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
