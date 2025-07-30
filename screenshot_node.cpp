#include <napi.h>
#include <Windows.h>
#include <gdiplus.h>
#include <memory>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

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
}

// 截图区域函数
Napi::Value CaptureScreen(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 4) {
        Napi::TypeError::New(env, "Expected 4 arguments: x, y, width, height")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    int x = info[0].As<Napi::Number>().Int32Value();
    int y = info[1].As<Napi::Number>().Int32Value();
    int width = info[2].As<Napi::Number>().Int32Value();
    int height = info[3].As<Napi::Number>().Int32Value();

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
        Napi::TypeError::New(env, "Failed to capture screen").ThrowAsJavaScriptException();
        return env.Null();
    }

    // 获取图像数据
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // 负值表示从上到下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    int imageSize = width * height * 4;
    unsigned char* imageData = new unsigned char[imageSize];

    GetDIBits(hDC, hBitmap, 0, height, imageData, &bmi, DIB_RGB_COLORS);

    SelectObject(hDC, oldObj);
    DeleteDC(hDC);
    DeleteObject(hBitmap);
    ReleaseDC(NULL, hScreen);

    // 创建Node.js Buffer对象
    Napi::Buffer<unsigned char> buffer = Napi::Buffer<unsigned char>::New(
        env, imageData, imageSize, [](Napi::Env env, unsigned char* data) {
            delete[] data;
        });

    // 返回对象包含图像数据和尺寸信息
    Napi::Object result_obj = Napi::Object::New(env);
    result_obj.Set("data", buffer);
    result_obj.Set("width", Napi::Number::New(env, width));
    result_obj.Set("height", Napi::Number::New(env, height));
    result_obj.Set("stride", Napi::Number::New(env, width * 4));

    return result_obj;
}

// 全屏截图函数
Napi::Value CaptureFullScreen(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // 创建参数数组并调用CaptureScreen
    Napi::Value args[] = {
        Napi::Number::New(env, x),
        Napi::Number::New(env, y),
        Napi::Number::New(env, width),
        Napi::Number::New(env, height)
    };

    Napi::CallbackInfo new_info(env, info.GetNewTarget(), 4, args, info.Data());
    return CaptureScreen(new_info);
}

// 保存到PNG文件
Napi::Value SaveToPNG(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Expected 2 arguments: imageObject, filepath")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object imageObj = info[0].As<Napi::Object>();
    std::string filepath = info[1].As<Napi::String>().Utf8Value();

    Napi::Buffer<unsigned char> buffer = imageObj.Get("data").As<Napi::Buffer<unsigned char>>();
    int width = imageObj.Get("width").As<Napi::Number>().Int32Value();
    int height = imageObj.Get("height").As<Napi::Number>().Int32Value();
    int stride = imageObj.Get("stride").As<Napi::Number>().Int32Value();

    initGdiPlus();

    // 创建GDI+ Bitmap
    Bitmap bitmap(width, height, stride, PixelFormat32bppARGB, buffer.Data());

    // 获取PNG编码器
    CLSID pngClsid;
    GetEncoderClsid(L"image/png", &pngClsid);

    // 转换文件路径为宽字符
    int wideSize = MultiByteToWideChar(CP_UTF8, 0, filepath.c_str(), -1, NULL, 0);
    wchar_t* widePath = new wchar_t[wideSize];
    MultiByteToWideChar(CP_UTF8, 0, filepath.c_str(), -1, widePath, wideSize);

    Status status = bitmap.Save(widePath, &pngClsid, NULL);
    delete[] widePath;

    return Napi::Boolean::New(env, status == Ok);
}

// 获取屏幕尺寸
Napi::Value GetScreenDimensions(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    Napi::Object result = Napi::Object::New(env);
    result.Set("width", Napi::Number::New(env, width));
    result.Set("height", Napi::Number::New(env, height));

    return result;
}

// 模块初始化
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("captureScreen", Napi::Function::New(env, CaptureScreen));
    exports.Set("captureFullScreen", Napi::Function::New(env, CaptureFullScreen));
    exports.Set("saveToPNG", Napi::Function::New(env, SaveToPNG));
    exports.Set("getScreenDimensions", Napi::Function::New(env, GetScreenDimensions));
    return exports;
}

NODE_API_MODULE(screenshot, Init) 