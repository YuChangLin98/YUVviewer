#pragma once

#include <QRect>
#include <QColor>

struct RectangleOverlay {
    QRect rect;
    QColor color = Qt::green;
    int id = 0;

    [[nodiscard]] static RectangleOverlay create(
        int x, int y, int w, int h, int id,
        const QColor &color = Qt::green);
};

inline RectangleOverlay RectangleOverlay::create(
    int x, int y, int w, int h, int id,
    const QColor &color)
{
    RectangleOverlay overlay;
    overlay.rect = QRect(x, y, qMax(1, w), qMax(1, h));
    overlay.color = color;
    overlay.id = id;
    return overlay;
}
