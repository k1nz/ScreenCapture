# ScreenCapture DLL & Node.js Addon

这是一个从ScreenCapture项目提取的纯Windows API截图库，提供DLL和Node.js addon两种形式。

## 特性

- ✅ **无Qt依赖** - 使用纯Windows API + GDI+
- ✅ **轻量级** - 只保留核心截图功能
- ✅ **高性能** - 直接调用系统API
- ✅ **多种输出** - 支持文件保存和剪贴板
- ✅ **跨屏支持** - 支持多显示器环境

## 编译要求

- Windows 10/11
- Visual Studio 2019/2022
- Windows SDK
- Node.js (仅Node.js addon需要)

## 方案一：DLL版本

### 编译DLL

```cmd
# 使用Visual Studio Developer Command Prompt
cl /LD /DSCREENSHOT_DLL_EXPORTS screenshot_dll.cpp /link gdiplus.lib user32.lib gdi32.lib
```

### 使用DLL (C++)

```cpp
#include "screenshot_dll.h"

int main() {
    // 全屏截图
    ScreenshotImage* fullscreen = captureFullScreen();
    if (fullscreen) {
        saveImageToPNG(fullscreen, "fullscreen.png");
        freeImage(fullscreen);
    }
    
    // 区域截图
    ScreenshotImage* area = captureScreen(100, 100, 800, 600);
    if (area) {
        saveImageToClipboard(area);
        freeImage(area);
    }
    
    return 0;
}
```

### 使用DLL (C#)

```csharp
using System;
using System.Runtime.InteropServices;

public class ScreenshotAPI {
    [DllImport("screenshot_dll.dll")]
    public static extern IntPtr captureScreen(int x, int y, int width, int height);
    
    [DllImport("screenshot_dll.dll")]
    public static extern IntPtr captureFullScreen();
    
    [DllImport("screenshot_dll.dll")]
    public static extern int saveImageToPNG(IntPtr image, string filepath);
    
    [DllImport("screenshot_dll.dll")]
    public static extern void freeImage(IntPtr image);
}
```

## 方案二：Node.js Addon

### 安装依赖

```bash
npm install
```

### 编译Addon

```bash
npm run build
```

### 使用Addon

```javascript
const screenshot = require('./build/Release/screenshot.node');

// 获取屏幕尺寸
const dimensions = screenshot.getScreenDimensions();
console.log(`屏幕尺寸: ${dimensions.width} x ${dimensions.height}`);

// 全屏截图
const fullScreenImage = screenshot.captureFullScreen();
screenshot.saveToPNG(fullScreenImage, 'fullscreen.png');

// 区域截图
const areaImage = screenshot.captureScreen(0, 0, 800, 600);
screenshot.saveToPNG(areaImage, 'area.png');

// 访问原始像素数据
const data = areaImage.data; // Buffer，BGRA格式
console.log(`图像数据大小: ${data.length} bytes`);
```

## API 参考

### DLL接口

```c
// 截图函数
ScreenshotImage* captureScreen(int x, int y, int width, int height);
ScreenshotImage* captureFullScreen();

// 保存函数
int saveImageToPNG(ScreenshotImage* image, const char* filepath);
int saveImageToClipboard(ScreenshotImage* image);

// 工具函数
void getScreenDimensions(int* width, int* height);
int isPointOnScreen(int x, int y);
void freeImage(ScreenshotImage* image);
```

### Node.js接口

```javascript
// 截图函数
const image = screenshot.captureScreen(x, y, width, height);
const image = screenshot.captureFullScreen();

// 保存函数
const success = screenshot.saveToPNG(image, filepath);

// 工具函数
const dimensions = screenshot.getScreenDimensions();
```

### 图像对象结构

```javascript
{
  data: Buffer,      // BGRA格式像素数据
  width: number,     // 图像宽度
  height: number,    // 图像高度
  stride: number     // 每行字节数 (width * 4)
}
```

## 性能对比

| 方案 | 文件大小 | 启动时间 | 内存占用 | 依赖 |
|------|----------|----------|----------|------|
| 原项目 | 8MB | 慢 | 高 | Qt 6.9.1 |
| DLL版本 | ~200KB | 快 | 低 | 仅GDI+ |
| Node.js版本 | ~300KB | 快 | 中等 | Node.js + GDI+ |

## 注意事项

1. **权限**：需要足够权限访问屏幕内容
2. **多显示器**：自动处理虚拟屏幕坐标
3. **颜色格式**：输出BGRA格式，注意字节序
4. **内存管理**：DLL版本需手动释放内存，Node.js版本自动管理
5. **线程安全**：GDI+初始化是线程安全的

## 与原项目对比

| 功能 | 原项目 | DLL/Node版本 |
|------|--------|--------------|
| 基础截图 | ✅ | ✅ |
| 区域截图 | ✅ | ✅ |
| 全屏截图 | ✅ | ✅ |
| 保存PNG | ✅ | ✅ |
| 保存剪贴板 | ✅ | ✅ (仅DLL) |
| 长截图 | ✅ | ❌ |
| 图形标注 | ✅ | ❌ |
| GUI界面 | ✅ | ❌ |
| 热键支持 | ✅ | ❌ |

## 测试

```bash
# Node.js版本测试
npm test

# 查看生成的截图文件
ls *.png
``` 