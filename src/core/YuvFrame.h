#pragma once

#include "YuvFormat.h"
#include <QByteArray>

struct YuvFrame {
    QByteArray yPlane;
    QByteArray uPlane;
    QByteArray vPlane;
    int width = 0;
    int height = 0;
    YuvFormat format = YuvFormat::YV12;

    [[nodiscard]] bool isValid() const noexcept;
};

inline bool YuvFrame::isValid() const noexcept
{
    if (width <= 0 || height <= 0)
        return false;
    if ((width & 1) || (height & 1))
        return false;

    qsizetype ySize = static_cast<qsizetype>(width) * height;
    qsizetype uvSize = (static_cast<qsizetype>(width) * height) / 4;

    if (yPlane.size() < ySize)
        return false;

    switch (format) {
    case YuvFormat::YV12:
        return uPlane.size() >= uvSize && vPlane.size() >= uvSize;
    case YuvFormat::NV12:
    case YuvFormat::NV21:
        return uPlane.size() >= uvSize * 2;
    }
    return false;
}

