#include "CoordinateInputPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QPushButton>
#include <QListWidget>
#include <QColorDialog>
#include <QMessageBox>

static QString textColorForBg(const QColor &bg)
{
    double lum = 0.299 * bg.redF() + 0.587 * bg.greenF() + 0.114 * bg.blueF();
    return lum > 0.5 ? QStringLiteral("black") : QStringLiteral("white");
}

CoordinateInputPanel::CoordinateInputPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);

    // Input group
    auto *inputGroup = new QGroupBox(QStringLiteral("添加矩形"));
    auto *inputLayout = new QFormLayout(inputGroup);

    m_xSpin = new QSpinBox;
    m_xSpin->setRange(0, 16384);
    inputLayout->addRow(QStringLiteral("X:"), m_xSpin);

    m_ySpin = new QSpinBox;
    m_ySpin->setRange(0, 16384);
    inputLayout->addRow(QStringLiteral("Y:"), m_ySpin);

    m_wSpin = new QSpinBox;
    m_wSpin->setRange(1, 16384);
    m_wSpin->setValue(100);
    inputLayout->addRow(QStringLiteral("宽度:"), m_wSpin);

    m_hSpin = new QSpinBox;
    m_hSpin->setRange(1, 16384);
    m_hSpin->setValue(100);
    inputLayout->addRow(QStringLiteral("高度:"), m_hSpin);

    mainLayout->addWidget(inputGroup);

    // Color picker
    auto *colorLayout = new QHBoxLayout;
    m_colorBtn = new QPushButton(QStringLiteral("颜色..."));
    m_colorBtn->setStyleSheet(
        QStringLiteral("background-color: %1; color: %2;")
            .arg(m_currentColor.name(), textColorForBg(m_currentColor)));
    connect(m_colorBtn, &QPushButton::clicked, this, &CoordinateInputPanel::onColorPicker);
    colorLayout->addWidget(m_colorBtn);
    mainLayout->addLayout(colorLayout);

    // Add button
    auto *addBtn = new QPushButton(QStringLiteral("添加矩形"));
    connect(addBtn, &QPushButton::clicked, this, &CoordinateInputPanel::onAddClicked);
    mainLayout->addWidget(addBtn);

    // Rectangle list
    auto *listGroup = new QGroupBox(QStringLiteral("已有矩形"));
    auto *listLayout = new QVBoxLayout(listGroup);

    m_listWidget = new QListWidget;
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, [this]() {
        QVector<int> ids;
        for (auto *item : m_listWidget->selectedItems())
            ids.append(item->data(Qt::UserRole).toInt());
        emit selectionChanged(ids);
    });
    listLayout->addWidget(m_listWidget);

    // List action buttons
    auto *listBtnLayout = new QHBoxLayout;

    auto *removeBtn = new QPushButton(QStringLiteral("删除选中"));
    connect(removeBtn, &QPushButton::clicked, this, &CoordinateInputPanel::onRemoveSelectedClicked);
    listBtnLayout->addWidget(removeBtn);

    auto *clearBtn = new QPushButton(QStringLiteral("清空全部"));
    clearBtn->setStyleSheet("QPushButton { color: #cc3333; }");
    connect(clearBtn, &QPushButton::clicked, this, &CoordinateInputPanel::onClearClicked);
    listBtnLayout->addWidget(clearBtn);

    listLayout->addLayout(listBtnLayout);

    mainLayout->addWidget(listGroup);
}

void CoordinateInputPanel::resetPanel()
{
    m_imageWidth = 0;
    m_imageHeight = 0;
    m_listWidget->clear();
    m_xSpin->setMaximum(16384);
    m_ySpin->setMaximum(16384);
    m_wSpin->setMaximum(16384);
    m_hSpin->setMaximum(16384);
}

void CoordinateInputPanel::onAddClicked()
{
    if (m_imageWidth <= 0 || m_imageHeight <= 0) {
        QMessageBox::warning(this, QStringLiteral("输入无效"),
                             QStringLiteral("请先加载图片。"));
        return;
    }

    int x = m_xSpin->value();
    int y = m_ySpin->value();
    int w = m_wSpin->value();
    int h = m_hSpin->value();
    int right = x + w;
    int bottom = y + h;
    if (x < 0 || y < 0 || x >= m_imageWidth || y >= m_imageHeight) {
        QMessageBox::warning(this, QStringLiteral("输入无效"),
                             QStringLiteral("矩形起点超出图片范围 (图片尺寸 %1 x %2)。")
                                 .arg(m_imageWidth).arg(m_imageHeight));
        return;
    }
    if (right > m_imageWidth || bottom > m_imageHeight) {
        QMessageBox::warning(this, QStringLiteral("输入无效"),
                             QStringLiteral("矩形超出图片范围。\n起点: (%1, %2)  矩形: %3 x %4\n图片右下角: (%5, %6)")
                                 .arg(x).arg(y).arg(w).arg(h)
                                 .arg(m_imageWidth).arg(m_imageHeight));
        return;
    }

    QRect rect(m_xSpin->value(), m_ySpin->value(), w, h);
    emit addRequested(rect, m_currentColor);
}

void CoordinateInputPanel::onRemoveSelectedClicked()
{
    QList<QListWidgetItem *> selected = m_listWidget->selectedItems();
    for (auto *item : selected) {
        int id = item->data(Qt::UserRole).toInt();
        emit removeRequested(id);
    }
}

void CoordinateInputPanel::onClearClicked()
{
    emit clearRequested();
}

void CoordinateInputPanel::onColorPicker()
{
    QColor color = QColorDialog::getColor(m_currentColor, this,
                                          QStringLiteral("选择矩形颜色"));
    if (color.isValid()) {
        m_currentColor = color;
        m_colorBtn->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2;")
                .arg(color.name(), textColorForBg(color)));
    }
}

void CoordinateInputPanel::onRectangleAdded(int id, const QRect &rect, const QColor &color)
{
    auto *item = new QListWidgetItem(
        QStringLiteral("矩形 %1: (%2, %3) %4x%5")
            .arg(id)
            .arg(rect.x())
            .arg(rect.y())
            .arg(rect.width())
            .arg(rect.height()));
    item->setData(Qt::UserRole, id);
    QPixmap px(16, 16);
    px.fill(color);
    item->setIcon(QIcon(px));
    m_listWidget->addItem(item);
}

void CoordinateInputPanel::onRectangleRemoved(int id)
{
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->data(Qt::UserRole).toInt() == id) {
            delete m_listWidget->takeItem(i);
            break;
        }
    }
}

void CoordinateInputPanel::onImageLoaded(int imageWidth, int imageHeight)
{
    m_imageWidth = imageWidth;
    m_imageHeight = imageHeight;
    m_xSpin->setMaximum(imageWidth - 1);
    m_ySpin->setMaximum(imageHeight - 1);
    m_wSpin->setMaximum(imageWidth);
    m_hSpin->setMaximum(imageHeight);
    m_listWidget->clear();
}
