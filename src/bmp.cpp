#include <Windows.h>
#include <iostream>
#include <climits>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <optional>
#include <utility>

#include "modest_clipper.h"

using namespace std;

void save_bitmap(const BITMAPINFOHEADER& bitmapHeader, const vector<unsigned char>& pixelBytes, int frameNum) {
    string fName = "_" + to_string(frameNum) + ".bmp";
    ofstream ofStream(fName, ios::binary);
    if (!ofStream) {
        return;
    }
    BITMAPFILEHEADER fHeader{};
    fHeader.bfType = 0x4D42; //windows bitmap
    fHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fHeader.bfSize = fHeader.bfOffBits + static_cast<DWORD>(pixelBytes.size());

    const char* fileHeaderBytePtr = reinterpret_cast<const char*>(&fHeader);
    const char* bitmapHeaderBytePtr = reinterpret_cast<const char*>(&bitmapHeader);
    const char* pixelBytesPtr = reinterpret_cast<const char*>(pixelBytes.data());
    streamsize pixelBytesStreamSize = static_cast<streamsize>(pixelBytes.size());

    ofStream.write(fileHeaderBytePtr, sizeof(fHeader));
    ofStream.write(bitmapHeaderBytePtr, sizeof(bitmapHeader));
    ofStream.write(pixelBytesPtr, pixelBytesStreamSize);

    if (!ofStream) {
        cerr << "bitmap write: fail\n";
        return;
    }
    cout << "bitmap write: success\n";
}
BITMAPINFO create_bitmap_info(int screenWidth, int screenHeight, int bitsPerPixel) {
    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = screenWidth;
    bitmapInfo.bmiHeader.biHeight = -screenHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = bitsPerPixel;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    return bitmapInfo;
}
optional<Frame> extract_frame_from_bitmap(HDC screenDc, HBITMAP screenBitmap, int screenWidth, int screenHeight, int bitsPerPixel)
{
    BITMAPINFO bitmapInfo = create_bitmap_info(screenWidth, screenHeight, bitsPerPixel);
    Frame frame{};
    frame.bitmapHeader = bitmapInfo.bmiHeader;
    frame.pixelBytes.resize(screenWidth * screenHeight * bitsPerPixel / CHAR_BIT);
    int copiedScanLines = GetDIBits(screenDc, screenBitmap, 0, screenHeight, frame.pixelBytes.data(), &bitmapInfo, DIB_RGB_COLORS);
    if (copiedScanLines != screenHeight) {
        return nullopt;
    }
    return frame;
}