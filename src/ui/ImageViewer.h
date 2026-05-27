#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QRect>
#include <QVector>
#include <QColor>
#include "RectangleOverlay.h"

class ImageViewer : public QWidget {
    Q_OBJECT

public:
    enum class Rotation { Deg0, Deg90, Deg180, Deg270 };
    enum class Origin { TopLeft, TopRight, BottomLeft, BottomRight };

    explicit ImageViewer(QWidget *parent = nullptr);

    void setImage(const QImage &image);

    int addRectangle(const QRect &rect, const QColor &color = Qt::green);
    void removeRectangle(int id);
    void clearRectangles();

    void setZoom(double factor);
    void zoomIn();
    void zoomOut();
    void zoomFit();
    [[nodiscard]] double zoomFactor() const { return m_zoomFactor; }

    void rotateLeft();
    void rotateRight();
    void resetRotation();
    [[nodiscard]] Rotation rotation() const { return m_rotation; }

    void setOrigin(Origin origin);
    [[nodiscard]] Origin origin() const { return m_origin; }

    void setSelectedRectangles(const QVector<int> &ids);
    void setScrollArea(QScrollArea *sa) { m_scrollArea = sa; }

    [[nodiscard]] QRect displayRectToImage(const QRect &displayRect) const;

signals:
    void rectangleAdded(int id, const QRect &imageRect, const QColor &color);
    void rectangleRemoved(int id);
    void mousePositionChanged(const QPoint &imagePos);
    void zoomChanged(double factor);
    void rotationChanged(int angle);
    void originChanged(int origin);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    QSize sizeHint() const override;

private:
    void updateGeometryFromImage();
    void invalidateRulerCache();
    [[nodiscard]] QPoint rulerOffset() const;
    QPoint widgetToImage(const QPoint &widgetPos) const;
    QPoint imageToDisplay(const QPoint &imagePos) const;

    QImage m_image;
    QVector<RectangleOverlay> m_rectangles;
    QVector<int> m_selectedIds;
    double m_zoomFactor = 1.0;
    int m_nextRectId = 1;
    Rotation m_rotation = Rotation::Deg0;
    Origin m_origin = Origin::TopLeft;

    QScrollArea *m_scrollArea = nullptr;

    QPixmap m_rulerCache;
    bool m_rulerCacheValid = false;

    static constexpr int kRulerSize = 20;
};
