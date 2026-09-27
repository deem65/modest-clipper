#pragma once

#include <Windows.h>
#include <vector>
#include <optional>
#include <array>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <string_view>
#include <utility>
#include <atomic>
#include <mfidl.h>
#include <mftransform.h>

class Encoder
{
public:
    Encoder() = default;
    ~Encoder();

    Encoder(const Encoder&) = delete;
    Encoder& operator=(const Encoder&) = delete;
    Encoder(Encoder&&) = delete;
    Encoder& operator=(Encoder&&) = delete;

    bool init();

private:
    void shutdown() noexcept;
    bool init_com();
    bool init_mf();
    bool create_hardware_encoder();

    Microsoft::WRL::ComPtr<IMFTransform> transform;
    Microsoft::WRL::ComPtr<IMFActivate> activation;

    bool comStarted{};
    bool mfStarted{};
};

inline std::atomic<bool> clip_running{ true };

void capture_clip();

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
void capture_loop();
bool get_window_dc(HDC& windowDc, HWND window);
bool get_memory_dc(HDC& memoryDc, HDC screenDc);
bool get_window_dimensions(HWND window, int& width, int& height);
bool get_window_bitmap(HBITMAP& windowBitmap, HDC windowDc, int width, int height);
bool check_hr(HRESULT hr, std::string_view m);

BITMAPINFO create_bitmap_info(int screenWidth, int screenHeight, int bitsPerPixel);

std::optional<Frame> capture_frame(FrameContext& context, int bits);

std::optional<Frame> extract_frame_from_bitmap(HDC screenDc, HBITMAP screenBitmap, int screenWidth, int screenHeight, int bitsPerPixel);

struct dcfg {
    IDXGIAdapter* adapter = nullptr;
    D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
    HMODULE software = nullptr;
    UINT flags = 0;
    std::array<D3D_FEATURE_LEVEL, 1> featureLvls{ D3D_FEATURE_LEVEL_11_0 };
    UINT sdk = D3D11_SDK_VERSION;
    D3D_FEATURE_LEVEL selectedFeatureLvl{};
};

struct DxgiFrame
{
    DXGI_OUTDUPL_FRAME_INFO info{};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication;

    DxgiFrame() = default;

    DxgiFrame(const DxgiFrame&) = delete;
    DxgiFrame& operator=(const DxgiFrame&) = delete;

    //std::move constructor
    DxgiFrame(DxgiFrame&& other) noexcept
        : info(other.info),
        texture(std::move(other.texture)),
        duplication(std::move(other.duplication))
    {
        other.info = {};
    }

    //std::move assignment
    DxgiFrame& operator=(DxgiFrame&& other) noexcept
    {
        if (this == &other)
            return *this;

        release();

        info = other.info;
        texture = std::move(other.texture);
        duplication = std::move(other.duplication);

        other.info = {};

        return *this;
    }

    void end() noexcept
    {
        release();
        info = {};
    }

    ~DxgiFrame()
    {
        release();
    }

private:
    void release() noexcept
    {
        texture.Reset();
        if (duplication.Get() != nullptr) {
            duplication->ReleaseFrame();
            duplication.Reset();
        }
    }
};
void inspect_frame(const DxgiFrame& frame);

class Dxgi
{
public:
    bool init();
    bool try_get_frame(DxgiFrame& frame, UINT timeoutMs = 100);

private:
    bool acquire_frame(
        DxgiFrame& frame,
        Microsoft::WRL::ComPtr<IDXGIResource>& resource,
        UINT timeoutMs
    );

    bool get_frame_texture(
        DxgiFrame& frame,
        Microsoft::WRL::ComPtr<IDXGIResource>& resource
    );

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> dctx;

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIOutput> output;
    Microsoft::WRL::ComPtr<IDXGIOutput1> output1;
    Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication;
};