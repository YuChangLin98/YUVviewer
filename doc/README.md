# YUV Viewer

基于 Qt 6.8 的桌面应用，用于查看原始 YUV 图片，支持矩形框标注。

## 功能

- 打开 **YV12**、**NV12**、**NV21** 格式的原始 YUV 文件（均为 YUV 4:2:0）
- 使用 BT.601 色彩转换将 YUV 转为 RGB 显示
- 缩放：放大/缩小（5% ~ 1000%）、适应窗口、Ctrl+滚轮缩放
- 通过输入坐标（X, Y, 宽, 高）来画矩形标注框
- 每个矩形框可单独设置颜色
- 矩形管理：面板添加、选中删除、一键清空
- 状态栏实时显示鼠标所在像素坐标

## 编译环境

- Qt 6.8+（MinGW 64-bit 套件）
- CMake 3.30+
- g++（MinGW 13.1+）

## 编译

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="<Qt6.8.3安装路径>/mingw_64"
cmake --build .
```

本机示例（Qt 安装在 `D:\My_PC_APK\Qt`）：

```bash
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/My_PC_APK/Qt/6.8.3/mingw_64"
```

## 使用方法

1. **文件 > 打开**（Ctrl+O）—— 选择 YUV 文件，设置格式和分辨率
2. 使用右侧 **矩形坐标** 面板：
   - 输入 X、Y、宽度、高度
   - 可选：选择颜色
   - 点击 **添加矩形**
3. 在列表中选中矩形，点击 **删除选中** 可移除
4. 通过 **视图** 菜单或工具栏控制缩放

## 项目结构

```
YuvViewer/
├── CMakeLists.txt
├── doc/
│   └── README.md
└── src/
    ├── main.cpp                          # 入口
    ├── core/
    │   ├── YuvFormat.h                   # YV12 / NV12 / NV21 枚举
    │   ├── YuvFrame.h                    # YUV 平面数据容器
    │   ├── YuvParser.h/.cpp              # 文件读取 + 平面分离
    │   ├── YuvToRgbConverter.h/.cpp      # BT.601 YCbCr → RGB 转换
    │   └── RectangleOverlay.h            # 矩形标注数据结构
    └── ui/
        ├── MainWindow.h/.cpp             # 主窗口（菜单/工具栏/状态栏）
        ├── ImageViewer.h/.cpp            # 图片显示控件（缩放/滚动/标注渲染）
        ├── OpenYuvDialog.h/.cpp          # 打开 YUV 文件对话框
        └── CoordinateInputPanel.h/.cpp   # 坐标输入面板（右侧停靠）
```
