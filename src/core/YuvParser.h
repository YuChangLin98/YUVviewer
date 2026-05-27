#pragma once

#include "YuvFormat.h"
#include "YuvFrame.h"
#include <QString>

class QFile;

class YuvParser {
public:
    struct ParseResult {
        YuvFrame frame;
        QString errorMessage;

        [[nodiscard]] bool ok() const noexcept { return errorMessage.isEmpty(); }
    };

    [[nodiscard]] static ParseResult parse(
        QFile &file, int width, int height, YuvFormat format) noexcept;
};
