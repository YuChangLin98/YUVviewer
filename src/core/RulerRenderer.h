#pragma once

#include <QPainter>

namespace RulerRenderer {

enum class Origin { TopLeft, TopRight, BottomLeft, BottomRight };

static constexpr int kTickLen = 6;
static constexpr int kTickTextGap = 1;
static constexpr int kTickTextXOff = 3;
static constexpr int kYTickTextInset = 8;
static constexpr int kYLabelInset = 10;
static constexpr int kXLabelInset = 10;
static constexpr int kLabelGap = 2;
static constexpr int kTickTextYOff = 2;

static inline const QColor kBg{0x505050};
static inline const QColor kTick{0xCCCCCC};

void paint(QPainter &painter, Origin origin, double zoomFactor,
           int imgW, int imgH, int rulerSize);

} // namespace RulerRenderer
