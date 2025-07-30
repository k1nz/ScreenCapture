#include "screenshot_dll.h"
#include <Windows.h>
#include <gdiplus.h>
#include <memory>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// 函数声明
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

namespace {
    bool gdiPlusInitialized = false;
    ULONG_PTR gdiplusToken;

    void initGdiPlus() {
        if (!gdiPlusInitialized) {
            GdiplusStartupInput gdiplusStartupInput;
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
            gdiPlusInitialized = true;
        }
    }
}

extern "C" {

ScreenshotImage* captureScreen(int x, int y, int width, int height) {
    initGdiPlus();

    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ oldObj = SelectObject(hDC, hBitmap);
    
    BOOL result = BitBlt(hDC, 0, 0, width, height, hScreen, x, y, SRCCOPY);
    
    if (!result) {
        SelectObject(hDC, oldObj);
        DeleteDC(hDC);
        DeleteObject(hBitmap);
        ReleaseDC(NULL, hScreen);
        return nullptr;
    }

    // 获取图像数据
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // 负值表示从上到下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    ScreenshotImage* image = new ScreenshotImage();
    image->width = width;
    image->height = height;
    image->stride = width * 4;
    image->data = new unsigned char[width * height * 4];

    GetDIBits(hDC, hBitmap, 0, height, image->data, &bmi, DIB_RGB_COLORS);

    SelectObject(hDC, oldObj);
    DeleteDC(hDC);
    DeleteObject(hBitmap);
    ReleaseDC(NULL, hScreen);

    return image;
}

ScreenshotImage* captureFullScreen() {
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    
    return captureScreen(x, y, width, height);
}

void freeImage(ScreenshotImage* image) {
    if (image) {
        delete[] image->data;
        delete image;
    }
}

int saveImageToPNG(ScreenshotImage* image, const char* filepath) {
    if (!image || !image->data) return 0;

    initGdiPlus();

    // 创建GDI+ Bitmap
    Bitmap bitmap(image->width, image->height, image->stride, 
                 PixelFormat32bppARGB, image->data);

    // 获取PNG编码器
    CLSID pngClsid;
    GetEncoderClsid(L"image/png", &pngClsid);

    // 转换文件路径为宽字符
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, filepath, -1, NULL, 0);
    wchar_t* widePath = new wchar_t[wideSize];
    MultiByteToWideChar(CP_UTF8, 0, filepath, -1, widePath, wideSize);

    Status status = bitmap.Save(widePath, &pngClsid, NULL);
    delete[] widePath;

    return (status == Ok) ? 1 : 0;
}

int saveImageToClipboard(ScreenshotImage* image) {
    if (!image || !image->data) return 0;

    HDC screenDC = GetDC(NULL);
    HDC memoryDC = CreateCompatibleDC(screenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(screenDC, image->width, image->height);
    HGDIOBJ oldObj = SelectObject(memoryDC, hBitmap);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = image->width;
    bmi.bmiHeader.biHeight = -image->height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    SetDIBitsToDevice(memoryDC, 0, 0, image->width, image->height, 
                     0, 0, 0, image->height, image->data, &bmi, DIB_RGB_COLORS);

    if (!OpenClipboard(NULL)) {
        SelectObject(memoryDC, oldObj);
        DeleteDC(memoryDC);
        DeleteObject(hBitmap);
        ReleaseDC(NULL, screenDC);
        return 0;
    }

    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();

    SelectObject(memoryDC, oldObj);
    DeleteDC(memoryDC);
    ReleaseDC(NULL, screenDC);

    return 1;
}

void getScreenDimensions(int* width, int* height) {
    *width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    *height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

int isPointOnScreen(int x, int y) {
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int screenW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenH = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    
    return (x >= screenX && x < screenX + screenW && 
            y >= screenY && y < screenY + screenH) ? 1 : 0;
}

} // extern "C"

// GDI+ 编码器辅助函数
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    return -1;
} 