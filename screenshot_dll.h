#pragma once

#ifdef SCREENSHOT_DLL_EXPORTS
#define SCREENSHOT_API __declspec(dllexport)
#else
#define SCREENSHOT_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 结构体定义
typedef struct {
    unsigned char* data;  // BGRA格式图像数据
    int width;
    int height;
    int stride;          // 每行字节数
} ScreenshotImage;

// 基础截图功能
SCREENSHOT_API ScreenshotImage* captureScreen(int x, int y, int width, int height);
SCREENSHOT_API ScreenshotImage* captureFullScreen();
SCREENSHOT_API void freeImage(ScreenshotImage* image);

// 保存功能
SCREENSHOT_API int saveImageToPNG(ScreenshotImage* image, const char* filepath);
SCREENSHOT_API int saveImageToClipboard(ScreenshotImage* image);

// 工具函数
SCREENSHOT_API void getScreenDimensions(int* width, int* height);
SCREENSHOT_API int isPointOnScreen(int x, int y);

#ifdef __cplusplus
}
#endif 