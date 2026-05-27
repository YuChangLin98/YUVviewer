#include "ImageViewer.h"
#include "RulerRenderer.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollArea>
#include <QtMath>

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(100, 100);
}

void ImageViewer::setImage(const QImage &image)
{
    m_image = image;
    m_rectangles.clear();
    m_nextRectId = 1;
    m_rotation = Rotation::Deg0;
    if (m_origin != Origin::TopLeft) {
        m_origin = Origin::TopLeft;
        emit originChanged(static_cast<int>(m_origin));
    }
    invalidateRulerCache();
    updateGeometryFromImage();
    update();
    emit rotationChanged(0);
}

int ImageViewer::addRectangle(const QRect &rect, const QColor &color)
{
    QRect clamped = rect.intersected(m_image.rect());
    if (clamped.isEmpty())
        return -1;

    int id = m_nextRectId++;
    auto overlay = RectangleOverlay::create(
        clamped.x(), clamped.y(), clamped.width(), clamped.height(),
        id, color);
    m_rectangles.append(overlay);
    update();
    emit rectangleAdded(id, clamped, color);
    return id;
}

void ImageViewer::removeRectangle(int id)
{
    auto it = std::find_if(m_rectangles.begin(), m_rectangles.end(),
                           [id](const RectangleOverlay &r) { return r.id == id; });
    if (it != m_rectangles.end()) {
        m_rectangles.erase(it);
        update();
        emit rectangleRemoved(id);
    }
}

void ImageViewer::clearRectangles()
{
    if (m_rectangles.isEmpty())
        return;
    QVector<int> ids;
    ids.reserve(m_rectangles.size());
    for (const auto &r : m_rectangles)
        ids.append(r.id);
    m_rectangles.clear();
    m_nextRectId = 1;
    for (int id : ids)
        emit rectangleRemoved(id);
    update();
}

void ImageViewer::setZoom(double factor)
{
    factor = qBound(0.05, factor, 10.0);
    if (qFuzzyCompare(factor, m_zoomFactor))
        return;
    m_zoomFactor = factor;
    invalidateRulerCache();
    updateGeometryFromImage();
    update();
    emit zoomChanged(m_zoomFactor);
}

void ImageViewer::zoomIn()
{
    setZoom(m_zoomFactor * 1.25);
}

void ImageViewer::zoomOut()
{
    setZoom(m_zoomFactor / 1.25);
}

void ImageViewer::zoomFit()
{
    if (m_image.isNull())
        return;

    QScrollArea *sa = m_scrollArea;
    if (!sa || !sa->viewport())
        return;

    QSize viewportSize = sa->viewport()->size();
    bool rotated = (m_rotation == Rotation::Deg90 || m_rotation == Rotation::Deg270);
    double dispW = rotated ? m_image.height() : m_image.width();
    double dispH = rotated ? m_image.width() : m_image.height();
    double fitW = static_cast<double>(viewportSize.width() - 2 * kRulerSize) / dispW;
    double fitH = static_cast<double>(viewportSize.height() - 2 * kRulerSize) / dispH;
    setZoom(qMin(fitW, fitH));
}

// --------------- Rotation ---------------

void ImageViewer::rotateLeft()
{
    m_rotation = static_cast<Rotation>((static_cast<int>(m_rotation) + 3) % 4);
    invalidateRulerCache();
    updateGeometryFromImage();
    update();
    emit rotationChanged(static_cast<int>(m_rotation) * 90);
}

void ImageViewer::rotateRight()
{
    m_rotation = static_cast<Rotation>((static_cast<int>(m_rotation) + 1) % 4);
    invalidateRulerCache();
    updateGeometryFromImage();
    update();
    emit rotationChanged(static_cast<int>(m_rotation) * 90);
}

void ImageViewer::resetRotation()
{
    if (m_rotation == Rotation::Deg0)
        return;
    m_rotation = Rotation::Deg0;
    invalidateRulerCache();
    updateGeometryFromImage();
    update();
    emit rotationChanged(0);
}

// --------------- Origin ---------------

void ImageViewer::setSelectedRectangles(const QVector<int> &ids)
{
    if (m_selectedIds != ids) {
        m_selectedIds = ids;
        update();
    }
}

void ImageViewer::setOrigin(Origin origin)
{
    if (m_origin == origin)
        return;
    m_origin = origin;
    invalidateRulerCache();
    update();
    emit originChanged(static_cast<int>(m_origin));
}

// --------------- Geometry ---------------

void ImageViewer::invalidateRulerCache()
{
    m_rulerCacheValid = false;
}

void ImageViewer::updateGeometryFromImage()
{
    QSize sz;
    if (!m_image.isNull()) {
        bool rotated = (m_rotation == Rotation::Deg90 || m_rotation == Rotation::Deg270);
        int dispW = rotated ? m_image.height() : m_image.width();
        int dispH = rotated ? m_image.width() : m_image.height();
        sz = QSize(static_cast<int>(dispW * m_zoomFactor) + 2 * kRulerSize,
                   static_cast<int>(dispH * m_zoomFactor) + 2 * kRulerSize);
    } else {
        sz = QSize(100, 100);
    }
    setMinimumSize(sz);
    resize(sz);
}

QSize ImageViewer::sizeHint() const
{
    if (m_image.isNull())
        return QSize(400, 300);
    bool rotated = (m_rotation == Rotation::Deg90 || m_rotation == Rotation::Deg270);
    int dispW = rotated ? m_image.height() : m_image.width();
    int dispH = rotated ? m_image.width() : m_image.height();
    return QSize(static_cast<int>(dispW * m_zoomFactor) + 2 * kRulerSize,
                 static_cast<int>(dispH * m_zoomFactor) + 2 * kRulerSize);
}

// --------------- Ruler Positioning ---------------

QPoint ImageViewer::rulerOffset() const
{
    return QPoint(kRulerSize, kRulerSize);
}

// --------------- Paint ---------------

void ImageViewer::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), QColor("#2D2D2D"));

    if (m_image.isNull()) {
        painter.setPen(QColor("#888888"));
        painter.drawText(rect(), Qt::AlignCenter,
                         QStringLiteral("未加载图片。\n请使用 文件 > 打开 加载 YUV 图片。"));
        return;
    }

    int imgW = m_image.width();
    int imgH = m_image.height();
    bool rotated = (m_rotation == Rotation::Deg90 || m_rotation == Rotation::Deg270);
    double dispW = (rotated ? imgH : imgW) * m_zoomFactor;
    double dispH = (rotated ? imgW : imgH) * m_zoomFactor;
    double angle = static_cast<int>(m_rotation) * 90.0;

    QPoint offset = rulerOffset();

    if (!m_rulerCacheValid || m_rulerCache.size() != size()) {
        m_rulerCache = QPixmap(size());
        m_rulerCache.fill(Qt::transparent);
        QPainter cp(&m_rulerCache);
        cp.translate(offset.x(), offset.y());
        if (angle != 0.0) {
            cp.translate(dispW / 2.0, dispH / 2.0);
            cp.rotate(angle);
            cp.translate(-imgW * m_zoomFactor / 2.0, -imgH * m_zoomFactor / 2.0);
        }
        RulerRenderer::paint(cp, static_cast<RulerRenderer::Origin>(m_origin),
                             m_zoomFactor, imgW, imgH, kRulerSize);
        m_rulerCacheValid = true;
    }

    painter.drawPixmap(0, 0, m_rulerCache);

    painter.save();
    painter.translate(offset.x(), offset.y());

    if (angle != 0.0) {
        painter.translate(dispW / 2.0, dispH / 2.0);
        painter.rotate(angle);
        painter.translate(-imgW * m_zoomFactor / 2.0, -imgH * m_zoomFactor / 2.0);
    }

    painter.scale(m_zoomFactor, m_zoomFactor);
    painter.drawImage(0, 0, m_image);

    for (const auto &overlay : m_rectangles) {
        QColor fillColor = overlay.color;
        fillColor.setAlpha(51);
        painter.fillRect(overlay.rect, fillColor);

        bool selected = m_selectedIds.contains(overlay.id);
        QPen pen(selected ? Qt::yellow : overlay.color,
                 selected ? 2.0 / m_zoomFactor : 1.0 / m_zoomFactor,
                 selected ? Qt::DashLine : Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(overlay.rect);

    }

    painter.restore();
}

// --------------- Coordinate Helpers ---------------

QPoint ImageViewer::widgetToImage(const QPoint &wp) const
{
    int imgW = m_image.width();
    int imgH = m_image.height();
    bool rotated = (m_rotation == Rotation::Deg90 || m_rotation == Rotation::Deg270);
    double dispW = (rotated ? imgH : imgW) * m_zoomFactor;
    double dispH = (rotated ? imgW : imgH) * m_zoomFactor;
    double angle = static_cast<int>(m_rotation) * 90.0;

    QPoint off = rulerOffset();
    double dx = wp.x() - off.x() - dispW / 2.0;
    double dy = wp.y() - off.y() - dispH / 2.0;

    double rad = qDegreesToRadians(-angle);
    double rx = dx * qCos(rad) - dy * qSin(rad);
    double ry = dx * qSin(rad) + dy * qCos(rad);

    int ix = static_cast<int>((rx + imgW * m_zoomFactor / 2.0) / m_zoomFactor);
    int iy = static_cast<int>((ry + imgH * m_zoomFactor / 2.0) / m_zoomFactor);

    return QPoint(ix, iy);
}

QPoint ImageViewer::imageToDisplay(const QPoint &imagePos) const
{
    if (imagePos.x() < 0 || imagePos.y() < 0)
        return imagePos;

    int dx = imagePos.x();
    int dy = imagePos.y();

    switch (m_origin) {
    case Origin::TopLeft:
        break;
    case Origin::TopRight:
        dx = m_image.width() - 1 - dx;
        break;
    case Origin::BottomLeft:
        dy = m_image.height() - 1 - dy;
        break;
    case Origin::BottomRight:
        dx = m_image.width() - 1 - dx;
        dy = m_image.height() - 1 - dy;
        break;
    }

    return QPoint(dx, dy);
}

QRect ImageViewer::displayRectToImage(const QRect &displayRect) const
{
    int x = displayRect.x();
    int y = displayRect.y();
    int w = displayRect.width();
    int h = displayRect.height();

    switch (m_origin) {
    case Origin::TopLeft:
        break;
    case Origin::TopRight:
        x = m_image.width() - (x + w);
        break;
    case Origin::BottomLeft:
        y = m_image.height() - (y + h);
        break;
    case Origin::BottomRight:
        x = m_image.width() - (x + w);
        y = m_image.height() - (y + h);
        break;
    }

    return QRect(x, y, w, h);
}

// --------------- Mouse ---------------

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint imgPos(-1, -1);
    int mx = event->pos().x();
    int my = event->pos().y();

    if (!m_image.isNull()) {
        int w = width();
        int h = height();
        bool inX = (mx >= kRulerSize && mx < w - kRulerSize);
        bool inY = (my >= kRulerSize && my < h - kRulerSize);
        if (inX && inY) {
            QPoint rawPos = widgetToImage(event->pos());
            if (rawPos.x() >= 0 && rawPos.x() < m_image.width() &&
                rawPos.y() >= 0 && rawPos.y() < m_image.height()) {
                imgPos = imageToDisplay(rawPos);
            }
        }
    }

    emit mousePositionChanged(imgPos);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        double delta = event->angleDelta().y();
        if (delta != 0) {
            setZoom(m_zoomFactor * (delta > 0 ? 1.15 : (1.0 / 1.15)));
        }
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}
