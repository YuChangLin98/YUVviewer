#pragma once

#include <QWidget>
#include <QRect>
#include <QColor>

class QSpinBox;
class QPushButton;
class QListWidget;

class CoordinateInputPanel : public QWidget {
    Q_OBJECT

public:
    explicit CoordinateInputPanel(QWidget *parent = nullptr);
    void resetPanel();

signals:
    void addRequested(const QRect &rect, const QColor &color);
    void removeRequested(int id);
    void clearRequested();
    void selectionChanged(const QVector<int> &ids);

public slots:
    void onRectangleAdded(int id, const QRect &rect, const QColor &color);
    void onRectangleRemoved(int id);
    void onImageLoaded(int imageWidth, int imageHeight);

private slots:
    void onAddClicked();
    void onClearClicked();
    void onRemoveSelectedClicked();
    void onColorPicker();

private:
    QSpinBox *m_xSpin = nullptr;
    QSpinBox *m_ySpin = nullptr;
    QSpinBox *m_wSpin = nullptr;
    QSpinBox *m_hSpin = nullptr;
    QPushButton *m_colorBtn = nullptr;
    QColor m_currentColor = Qt::green;
    QListWidget *m_listWidget = nullptr;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
};
