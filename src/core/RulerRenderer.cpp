#include "RulerRenderer.h"

namespace RulerRenderer {

void paint(QPainter &painter, Origin origin, double zoomFactor,
           int imgW, int imgH, int rulerSize)
{
    double totalW = imgW * zoomFactor;
    double totalH = imgH * zoomFactor;

    bool isTop = (origin == Origin::TopLeft || origin == Origin::TopRight);
    bool isLeft = (origin == Origin::TopLeft || origin == Origin::BottomLeft);

    double xRuleY = isTop ? -rulerSize : totalH;
    double yRuleX = isLeft ? -rulerSize : totalW;
    double cornerX = yRuleX;
    double cornerY = xRuleY;

    // Corner with O mark
    painter.fillRect(QRectF(cornerX, cornerY, rulerSize, rulerSize), kBg);
    {
        QFont oFont("Consolas", 11);
        oFont.setItalic(true);
        oFont.setStyleHint(QFont::Monospace);
        painter.setFont(oFont);
        painter.setPen(Qt::white);
        painter.drawText(QRectF(cornerX, cornerY, rulerSize, rulerSize),
                         Qt::AlignCenter, QStringLiteral("O"));
    }

    // X ruler background
    painter.fillRect(QRectF(0, xRuleY, totalW, rulerSize), kBg);

    // Y ruler background
    painter.fillRect(QRectF(yRuleX, 0, rulerSize, totalH), kBg);

    int tickInterval;
    if (zoomFactor < 0.5)      tickInterval = 200;
    else if (zoomFactor < 1.0) tickInterval = 100;
    else if (zoomFactor < 2.0) tickInterval = 50;
    else                       tickInterval = 10;

    painter.setPen(kTick);
    QFont font("Consolas", 7);
    font.setStyleHint(QFont::Monospace);
    painter.setFont(font);

    // X axis ticks
    {
        double tickTop = isTop ? xRuleY + rulerSize - kTickLen : xRuleY;
        double tickBot = isTop ? xRuleY + rulerSize : xRuleY + kTickLen;
        double textY = isTop ? xRuleY + rulerSize - kTickTextGap
                             : xRuleY + rulerSize - kLabelGap;
        int labelStep = tickInterval * 5;

        for (int d = 0; d <= imgW; d += tickInterval) {
            int px = (d == imgW) ? (isLeft ? imgW : 0)
                                : (isLeft ? d : imgW - 1 - d);
            double wx = px * zoomFactor;
            painter.drawLine(QPointF(wx, tickTop), QPointF(wx, tickBot));
            if (d == imgW) {
                painter.drawText(QPointF(wx + kTickTextXOff, textY), QString::number(imgW));
            } else if (d > 0 && d % labelStep == 0) {
                painter.drawText(QPointF(wx + kTickTextXOff, textY), QString::number(d));
            }
        }
    }

    // X label at the far end
    {
        double labelX = isLeft ? totalW + kLabelGap : -kXLabelInset;
        double labelY = isTop ? xRuleY + rulerSize - kTickTextGap
                              : xRuleY + rulerSize - kLabelGap;
        QFont boldFont("Consolas", 9);
        boldFont.setBold(true);
        boldFont.setStyleHint(QFont::Monospace);
        painter.setFont(boldFont);
        painter.setPen(Qt::white);
        painter.drawText(QPointF(labelX, labelY), "X");
        painter.setFont(font);
        painter.setPen(kTick);
    }

    // Y axis ticks
    {
        double tickLeft = isLeft ? yRuleX + rulerSize - kTickLen : yRuleX;
        double tickRight = isLeft ? yRuleX + rulerSize : yRuleX + kTickLen;
        double textX = yRuleX + rulerSize - kYTickTextInset;
        int labelStep = tickInterval * 5;

        for (int d = 0; d <= imgH; d += tickInterval) {
            int py = (d == imgH) ? (isTop ? imgH : 0)
                                : (isTop ? d : imgH - 1 - d);
            double wy = py * zoomFactor;
            painter.drawLine(QPointF(tickLeft, wy), QPointF(tickRight, wy));
            if (d == imgH) {
                painter.save();
                painter.translate(textX, wy + kTickTextYOff);
                painter.rotate(-90);
                painter.drawText(QPoint(0, 0), QString::number(imgH));
                painter.restore();
            } else if (d > 0 && d % labelStep == 0) {
                painter.save();
                painter.translate(textX, wy + kTickTextYOff);
                painter.rotate(-90);
                painter.drawText(QPoint(0, 0), QString::number(d));
                painter.restore();
            }
        }
    }

    // Y label at the far end
    {
        double labelX = isLeft ? yRuleX + rulerSize - kYLabelInset : yRuleX + kLabelGap;
        double labelY = isTop ? totalH + kLabelGap : -kLabelGap;
        QFont boldFont("Consolas", 9);
        boldFont.setBold(true);
        boldFont.setStyleHint(QFont::Monospace);
        painter.setFont(boldFont);
        painter.setPen(Qt::white);
        painter.drawText(QPointF(labelX, labelY), "Y");
    }
}

} // namespace RulerRenderer
