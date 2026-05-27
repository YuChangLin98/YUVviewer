#include "YuvToRgbConverter.h"
#include <array>

static inline int clampToByte(int val)
{
    return val < 0 ? 0 : (val > 255 ? 255 : val);
}

// Precomputed BT.601 coefficient tables — initialized once, thread-safe.
namespace {
    struct YuvLut {
        std::array<int, 256> yRC;   // Y contribution (same for R, G, B)
        std::array<int, 256> vRC;   // V → R
        std::array<int, 256> uGC;   // U → G
        std::array<int, 256> vGC;   // V → G
        std::array<int, 256> uBC;   // U → B
    };

    YuvLut buildLut()
    {
        YuvLut lut;
        for (int i = 0; i < 256; ++i) {
            int y1 = i - 16;
            int c1 = i - 128;
            lut.yRC[i] = 1192 * y1;
            lut.vRC[i] = 1634 * c1;
            lut.uGC[i] = -401 * c1;
            lut.vGC[i] = -833 * c1;
            lut.uBC[i] = 2066 * c1;
        }
        return lut;
    }

    const YuvLut &lut()
    {
        static const YuvLut s_lut = buildLut();
        return s_lut;
    }
}

QImage YuvToRgbConverter::convert(const YuvFrame &frame) noexcept
{
    if (!frame.isValid())
        return {};

    int w = frame.width;
    int h = frame.height;
    QImage image(w, h, QImage::Format_ARGB32);
    if (image.isNull())
        return {};

    const auto &t = lut();
    const quint8 *yData = reinterpret_cast<const quint8 *>(frame.yPlane.constData());
    const quint8 *uData = reinterpret_cast<const quint8 *>(frame.uPlane.constData());
    const quint8 *vData = (frame.format == YuvFormat::YV12)
        ? reinterpret_cast<const quint8 *>(frame.vPlane.constData())
        : nullptr;

    int uvWidth = w / 2;

    for (int row = 0; row < h; ++row) {
        quint32 *scanLine = reinterpret_cast<quint32 *>(image.scanLine(row));
        int chromaRow = row / 2;
        int yRowBase = row * w;

        for (int col = 0; col < w; ++col) {
            int Y = yData[yRowBase + col];
            int chromaIdx = chromaRow * uvWidth + (col / 2);
            int U, V;

            switch (frame.format) {
            case YuvFormat::YV12:
                U = uData[chromaIdx];
                V = vData[chromaIdx];
                break;
            case YuvFormat::NV12:
                U = uData[chromaIdx * 2];
                V = uData[chromaIdx * 2 + 1];
                break;
            case YuvFormat::NV21:
                V = uData[chromaIdx * 2];
                U = uData[chromaIdx * 2 + 1];
                break;
            }

            int r = (t.yRC[Y] + t.vRC[V] + 512) >> 10;
            int g = (t.yRC[Y] + t.uGC[U] + t.vGC[V] + 512) >> 10;
            int b = (t.yRC[Y] + t.uBC[U] + 512) >> 10;

            scanLine[col] = 0xFF000000
                | (clampToByte(r) << 16)
                | (clampToByte(g) << 8)
                | clampToByte(b);
        }
    }

    return image;
}
