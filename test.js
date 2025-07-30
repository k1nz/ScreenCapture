const screenshot = require('./build/Release/screenshot.node');
const fs = require('fs');
const path = require('path');

console.log('截图模块测试开始...\n');

// 1. 获取屏幕尺寸
console.log('1. 获取屏幕尺寸:');
const dimensions = screenshot.getScreenDimensions();
console.log(`   屏幕尺寸: ${dimensions.width} x ${dimensions.height}\n`);

// 2. 全屏截图
console.log('2. 全屏截图测试:');
try {
    console.log('   正在进行全屏截图...');
    const fullScreenImage = screenshot.captureFullScreen();
    console.log(`   截图成功! 尺寸: ${fullScreenImage.width} x ${fullScreenImage.height}`);
    console.log(`   数据大小: ${fullScreenImage.data.length} bytes\n`);

    // 保存全屏截图
    const fullScreenPath = path.join(__dirname, 'fullscreen.png');
    const saveResult = screenshot.saveToPNG(fullScreenImage, fullScreenPath);
    if (saveResult) {
        console.log(`   全屏截图已保存到: ${fullScreenPath}\n`);
    } else {
        console.log('   保存全屏截图失败\n');
    }
} catch (error) {
    console.error('   全屏截图失败:', error.message, '\n');
}

// 3. 区域截图
console.log('3. 区域截图测试:');
try {
    console.log('   正在截取屏幕左上角 800x600 区域...');
    const areaImage = screenshot.captureScreen(0, 0, 800, 600);
    console.log(`   区域截图成功! 尺寸: ${areaImage.width} x ${areaImage.height}`);
    console.log(`   数据大小: ${areaImage.data.length} bytes\n`);

    // 保存区域截图
    const areaPath = path.join(__dirname, 'area.png');
    const saveAreaResult = screenshot.saveToPNG(areaImage, areaPath);
    if (saveAreaResult) {
        console.log(`   区域截图已保存到: ${areaPath}\n`);
    } else {
        console.log('   保存区域截图失败\n');
    }
} catch (error) {
    console.error('   区域截图失败:', error.message, '\n');
}

// 4. 使用示例：将截图数据转换为不同格式
console.log('4. 图像数据处理示例:');
try {
    const smallImage = screenshot.captureScreen(100, 100, 200, 150);
    
    // 模拟处理BGRA数据 (Windows默认格式)
    console.log('   处理图像数据...');
    const data = smallImage.data;
    let redSum = 0, greenSum = 0, blueSum = 0;
    
    for (let i = 0; i < data.length; i += 4) {
        // BGRA格式: [B, G, R, A]
        blueSum += data[i];
        greenSum += data[i + 1];
        redSum += data[i + 2];
    }
    
    const pixelCount = data.length / 4;
    console.log(`   图像平均颜色值:`);
    console.log(`   - Red: ${Math.round(redSum / pixelCount)}`);
    console.log(`   - Green: ${Math.round(greenSum / pixelCount)}`);
    console.log(`   - Blue: ${Math.round(blueSum / pixelCount)}\n`);
} catch (error) {
    console.error('   图像数据处理失败:', error.message, '\n');
}

console.log('测试完成!');
console.log('\n使用说明:');
console.log('- captureScreen(x, y, width, height): 截取指定区域');
console.log('- captureFullScreen(): 全屏截图');
console.log('- saveToPNG(imageObject, filepath): 保存为PNG文件');
console.log('- getScreenDimensions(): 获取屏幕尺寸');
console.log('\n返回的图像对象包含:');
console.log('- data: Buffer，包含BGRA格式的像素数据');
console.log('- width: 图像宽度');
console.log('- height: 图像高度');
console.log('- stride: 每行字节数'); 