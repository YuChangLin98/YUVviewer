#pragma once

#include "YuvFrame.h"
#include <QImage>

class YuvToRgbConverter {
public:
    [[nodiscard]] static QImage convert(const YuvFrame &frame) noexcept;
};
