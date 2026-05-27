#include "YuvParser.h"
#include <QFile>

YuvParser::ParseResult YuvParser::parse(
    QFile &file, int width, int height, YuvFormat format) noexcept
{
    ParseResult result;
    result.frame.format = format;

    if (width <= 0 || height <= 0) {
        result.errorMessage = QString("Invalid dimensions: %1 x %2").arg(width).arg(height);
        return result;
    }

    if ((width & 1) || (height & 1)) {
        result.errorMessage =
            QString("Width and height must be even for 4:2:0 formats (got %1 x %2)")
                .arg(width).arg(height);
        return result;
    }

    qint64 expectedSize = static_cast<qint64>(width) * height * 3 / 2;
    if (file.size() < expectedSize) {
        result.errorMessage =
            QString("File size %1 bytes, expected at least %2 bytes (%3 x %4, %5 format)")
                .arg(file.size())
                .arg(expectedSize)
                .arg(width)
                .arg(height)
                .arg(format == YuvFormat::YV12 ? "YV12" :
                      format == YuvFormat::NV12 ? "NV12" : "NV21");
        return result;
    }

    QByteArray rawData = file.read(expectedSize);
    if (rawData.size() < expectedSize) {
        result.errorMessage = QString("Failed to read file data.");
        return result;
    }

    result.frame.width = width;
    result.frame.height = height;

    int ySize = width * height;
    int uvSize = (width * height) / 4;

    result.frame.yPlane = rawData.left(ySize);

    switch (format) {
    case YuvFormat::YV12: {
        int vOffset = ySize;
        int uOffset = ySize + uvSize;
        result.frame.vPlane = rawData.mid(vOffset, uvSize);
        result.frame.uPlane = rawData.mid(uOffset, uvSize);
        break;
    }
    case YuvFormat::NV12:
    case YuvFormat::NV21:
        result.frame.uPlane = rawData.mid(ySize, uvSize * 2);
        break;
    }

    return result;
}
