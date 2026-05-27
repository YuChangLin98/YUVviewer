#include "MainWindow.h"
#include "ImageViewer.h"
#include "OpenYuvDialog.h"
#include "CoordinateInputPanel.h"
#include "YuvFormat.h"
#include "YuvParser.h"
#include "YuvToRgbConverter.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QFile>
#include <QAction>
#include <QActionGroup>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("YUV 查看器"));
    setAcceptDrops(true);

    m_imageViewer = new ImageViewer;

    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_imageViewer);
    m_scrollArea->setStyleSheet("QScrollArea { background-color: #2D2D2D; }");
    setCentralWidget(m_scrollArea);
    m_imageViewer->setScrollArea(m_scrollArea);

    createMenus();
    createToolBar();
    createStatusBar();
    createDockPanel();
    updateActionStates(false);

    QSettings settings("YuvViewer", "YuvViewer");
    m_recentFiles = settings.value("recentFiles").toStringList();
    updateRecentMenu();

    m_convertWatcher = new QFutureWatcher<QImage>(this);
    connect(m_convertWatcher, &QFutureWatcher<QImage>::finished, this, [this]() {
        QImage image = m_convertWatcher->result();
        int gen = m_convertWatcher->property("openGen").toInt();
        if (!image.isNull() && gen == m_openGeneration) {
            m_imageViewer->setImage(image);
            m_imageViewer->zoomFit();
        }
    });

    connect(m_imageViewer, &ImageViewer::mousePositionChanged,
            this, &MainWindow::onMousePositionChanged);
    connect(m_imageViewer, &ImageViewer::zoomChanged, this, [this](double factor) {
        int pct = static_cast<int>(factor * 100.0);
        QString text = QStringLiteral("%1%").arg(pct);
        if (m_zoomCombo->currentText() != text) {
            m_zoomCombo->blockSignals(true);
            m_zoomCombo->setCurrentText(text);
            m_zoomCombo->blockSignals(false);
        }
    });
    connect(m_imageViewer, &ImageViewer::rotationChanged, this, [this](int angle) {
        statusBar()->showMessage(
            QStringLiteral("图片已旋转 %1°").arg(angle), 2000);
        bool atOrigin = (angle % 360 == 0);
        m_originCombo->setEnabled(atOrigin);
        if (m_originActionGroup)
            m_originActionGroup->setEnabled(atOrigin);
    });
    connect(m_imageViewer, &ImageViewer::originChanged, this, [this](int origin) {
        m_originCombo->blockSignals(true);
        m_originCombo->setCurrentIndex(origin);
        m_originCombo->blockSignals(false);
        if (m_originActionGroup) {
            const auto actions = m_originActionGroup->actions();
            if (origin >= 0 && origin < actions.size())
                actions[origin]->setChecked(true);
        }
    });
}

void MainWindow::createMenus()
{
    auto *fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));

    auto *openAction = fileMenu->addAction(QStringLiteral("打开(&O)...\tCtrl+O"));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onFileOpen);

    auto *closeAction = fileMenu->addAction(QStringLiteral("关闭图片(&C)\tCtrl+W"));
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    connect(closeAction, &QAction::triggered, this, &MainWindow::onCloseImage);
    m_closeAction = closeAction;
    m_imageActions.append(closeAction);

    fileMenu->addSeparator();

    m_recentMenu = fileMenu->addMenu(QStringLiteral("最近文件(&R)"));

    fileMenu->addSeparator();

    auto *exitAction = fileMenu->addAction(QStringLiteral("退出(&X)\tAlt+F4"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *viewMenu = menuBar()->addMenu(QStringLiteral("视图(&V)"));

    auto *zoomInAction = viewMenu->addAction(QStringLiteral("放大(&I)\tCtrl+="));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::onZoomIn);
    m_imageActions.append(zoomInAction);

    auto *zoomOutAction = viewMenu->addAction(QStringLiteral("缩小(&O)\tCtrl+-"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::onZoomOut);
    m_imageActions.append(zoomOutAction);

    auto *zoomFitAction = viewMenu->addAction(QStringLiteral("适应窗口(&F)\tCtrl+0"));
    zoomFitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(zoomFitAction, &QAction::triggered, this, &MainWindow::onZoomFit);
    m_imageActions.append(zoomFitAction);

    auto *resetZoomAction = viewMenu->addAction(QStringLiteral("原始大小(&R)\tCtrl+1"));
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    connect(resetZoomAction, &QAction::triggered, this, &MainWindow::onResetZoom);
    m_imageActions.append(resetZoomAction);

    viewMenu->addSeparator();

    auto *rotateLeftAction = viewMenu->addAction(QStringLiteral("逆时针旋转 90°(&L)\tCtrl+L"));
    rotateLeftAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::onRotateLeft);
    m_imageActions.append(rotateLeftAction);

    auto *rotateRightAction = viewMenu->addAction(QStringLiteral("顺时针旋转 90°(&R)\tCtrl+R"));
    rotateRightAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::onRotateRight);
    m_imageActions.append(rotateRightAction);

    auto *resetRotateAction = viewMenu->addAction(QStringLiteral("重置旋转(&T)\tCtrl+T"));
    resetRotateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(resetRotateAction, &QAction::triggered, this, &MainWindow::onResetRotation);
    m_imageActions.append(resetRotateAction);

    viewMenu->addSeparator();

    auto *originMenu = viewMenu->addMenu(QStringLiteral("坐标原点(&O)"));
    auto *originGroup = new QActionGroup(this);
    originGroup->setExclusive(true);
    m_originActionGroup = originGroup;

    auto addOriginAction = [&](const QString &text, int index) {
        auto *action = originMenu->addAction(text);
        action->setCheckable(true);
        if (index == 0) action->setChecked(true);
        originGroup->addAction(action);
        m_imageActions.append(action);
        connect(action, &QAction::triggered, this, [this, index]() {
            onOriginChanged(index);
        });
    };
    addOriginAction(QStringLiteral("左上"), 0);
    addOriginAction(QStringLiteral("右上"), 1);
    addOriginAction(QStringLiteral("左下"), 2);
    addOriginAction(QStringLiteral("右下"), 3);

    auto *helpMenu = menuBar()->addMenu(QStringLiteral("帮助(&H)"));

    auto *aboutAction = helpMenu->addAction(QStringLiteral("关于(&A)"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createToolBar()
{
    auto *toolbar = addToolBar(QStringLiteral("工具"));
    toolbar->setMovable(false);

    auto *openAction = toolbar->addAction(QStringLiteral("打开"));
    connect(openAction, &QAction::triggered, this, &MainWindow::onFileOpen);

    toolbar->addSeparator();

    auto *zoomInAction = toolbar->addAction(QStringLiteral("+"));
    zoomInAction->setToolTip(QStringLiteral("放大 (Ctrl+=)"));
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::onZoomIn);
    m_imageActions.append(zoomInAction);

    auto *zoomOutAction = toolbar->addAction(QStringLiteral("-"));
    zoomOutAction->setToolTip(QStringLiteral("缩小 (Ctrl+-)"));
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::onZoomOut);
    m_imageActions.append(zoomOutAction);

    auto *zoomFitAction = toolbar->addAction(QStringLiteral("适应"));
    zoomFitAction->setToolTip(QStringLiteral("适应窗口 (Ctrl+0)"));
    connect(zoomFitAction, &QAction::triggered, this, &MainWindow::onZoomFit);
    m_imageActions.append(zoomFitAction);

    m_zoomCombo = new QComboBox;
    m_zoomCombo->setEditable(true);
    m_zoomCombo->addItems({"25%", "50%", "75%", "100%", "150%", "200%", "400%",
                           QStringLiteral("适应")});
    m_zoomCombo->setCurrentText("100%");
    m_zoomCombo->setMinimumWidth(80);
    connect(m_zoomCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::onZoomChanged);
    connect(m_zoomCombo->lineEdit(), &QLineEdit::returnPressed, this, [this]() {
        onZoomChanged(0);
    });
    toolbar->addWidget(m_zoomCombo);

    toolbar->addSeparator();

    auto *rotateLeftAction = toolbar->addAction(QStringLiteral("↺"));
    rotateLeftAction->setToolTip(QStringLiteral("逆时针旋转 90° (Ctrl+L)"));
    connect(rotateLeftAction, &QAction::triggered, this, &MainWindow::onRotateLeft);
    m_imageActions.append(rotateLeftAction);

    auto *rotateRightAction = toolbar->addAction(QStringLiteral("↻"));
    rotateRightAction->setToolTip(QStringLiteral("顺时针旋转 90° (Ctrl+R)"));
    connect(rotateRightAction, &QAction::triggered, this, &MainWindow::onRotateRight);
    m_imageActions.append(rotateRightAction);

    auto *resetRotateAction = toolbar->addAction(QStringLiteral("⟲"));
    resetRotateAction->setToolTip(QStringLiteral("重置旋转 (Ctrl+T)"));
    connect(resetRotateAction, &QAction::triggered, this, &MainWindow::onResetRotation);
    m_imageActions.append(resetRotateAction);

    toolbar->addSeparator();

    m_originCombo = new QComboBox;
    m_originCombo->addItems({QStringLiteral("左上"), QStringLiteral("右上"),
                             QStringLiteral("左下"), QStringLiteral("右下")});
    m_originCombo->setCurrentIndex(0);
    m_originCombo->setToolTip(QStringLiteral("坐标原点"));
    connect(m_originCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onOriginChanged);
    toolbar->addWidget(m_originCombo);
}

void MainWindow::createStatusBar()
{
    m_imageInfoLabel = new QLabel(QStringLiteral("未加载图片"));
    statusBar()->addWidget(m_imageInfoLabel, 1);

    m_pixelPosLabel = new QLabel;
    statusBar()->addPermanentWidget(m_pixelPosLabel);
}

void MainWindow::createDockPanel()
{
    m_coordPanel = new CoordinateInputPanel;

    m_dockWidget = new QDockWidget(QStringLiteral("矩形坐标"), this);
    m_dockWidget->setWidget(m_coordPanel);
    m_dockWidget->setFeatures(QDockWidget::DockWidgetMovable |
                              QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, m_dockWidget);

    connect(m_coordPanel, &CoordinateInputPanel::addRequested, this,
            [this](const QRect &displayRect, const QColor &color) {
        QRect imageRect = m_imageViewer->displayRectToImage(displayRect);
        m_imageViewer->addRectangle(imageRect, color);
    });
    connect(m_coordPanel, &CoordinateInputPanel::removeRequested,
            m_imageViewer, &ImageViewer::removeRectangle);
    connect(m_coordPanel, &CoordinateInputPanel::clearRequested,
            m_imageViewer, &ImageViewer::clearRectangles);

    connect(m_imageViewer, &ImageViewer::rectangleAdded,
            m_coordPanel, &CoordinateInputPanel::onRectangleAdded);
    connect(m_imageViewer, &ImageViewer::rectangleRemoved,
            m_coordPanel, &CoordinateInputPanel::onRectangleRemoved);
    connect(m_coordPanel, &CoordinateInputPanel::selectionChanged,
            m_imageViewer, &ImageViewer::setSelectedRectangles);
}

void MainWindow::updateActionStates(bool hasImage)
{
    for (auto *a : m_imageActions)
        a->setEnabled(hasImage);
    m_zoomCombo->setEnabled(hasImage);
    m_originCombo->setEnabled(hasImage);
}

void MainWindow::onFileOpen()
{
    OpenYuvDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    openYuvFile(dlg.filePath(), dlg.imageWidth(), dlg.imageHeight(),
                static_cast<int>(dlg.yuvFormat()));
}

bool MainWindow::openYuvFile(const QString &filePath, int width, int height, int formatIndex)
{
    auto format = static_cast<YuvFormat>(formatIndex);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("无法打开文件:\n%1").arg(file.errorString()));
        return false;
    }

    auto result = YuvParser::parse(file, width, height, format);
    if (!result.ok()) {
        QMessageBox::warning(this, QStringLiteral("错误"),
                             QStringLiteral("解析 YUV 文件失败:\n%1").arg(result.errorMessage));
        return false;
    }

    m_imageViewer->setImage(QImage());
    m_coordPanel->resetPanel();
    statusBar()->showMessage(QStringLiteral("正在转换..."));

    YuvFrame frame = result.frame;
    int gen = ++m_openGeneration;
    m_convertWatcher->setProperty("openGen", gen);
    m_convertWatcher->setFuture(
        QtConcurrent::run([frame]() { return YuvToRgbConverter::convert(frame); }));

    m_coordPanel->onImageLoaded(width, height);

    const char *formatName = "YV12";
    switch (format) {
    case YuvFormat::YV12: formatName = "YV12"; break;
    case YuvFormat::NV12: formatName = "NV12"; break;
    case YuvFormat::NV21: formatName = "NV21"; break;
    }

    m_imageInfoLabel->setText(
        QStringLiteral("%1 | %2 x %3 | %4")
            .arg(QLatin1String(formatName))
            .arg(width)
            .arg(height)
            .arg(filePath));

    setWindowTitle(QStringLiteral("YUV 查看器 - %1").arg(filePath));
    addRecentFile(filePath);
    updateActionStates(true);
    return true;
}

void MainWindow::onCloseImage()
{
    m_openGeneration++;
    m_convertWatcher->cancel();
    m_imageViewer->setImage(QImage());
    m_coordPanel->resetPanel();
    m_imageInfoLabel->setText(QStringLiteral("未加载图片"));
    setWindowTitle(QStringLiteral("YUV 查看器"));
    updateActionStates(false);
}

void MainWindow::onZoomIn()  { m_imageViewer->zoomIn(); }
void MainWindow::onZoomOut() { m_imageViewer->zoomOut(); }
void MainWindow::onZoomFit() { m_imageViewer->zoomFit(); }
void MainWindow::onResetZoom() { m_imageViewer->setZoom(1.0); }

void MainWindow::onRotateLeft()  { m_imageViewer->rotateLeft(); }
void MainWindow::onRotateRight() { m_imageViewer->rotateRight(); }
void MainWindow::onResetRotation() { m_imageViewer->resetRotation(); }

void MainWindow::onOriginChanged(int index)
{
    auto origin = static_cast<ImageViewer::Origin>(index);
    m_imageViewer->setOrigin(origin);
    if (m_originCombo && m_originCombo->currentIndex() != index) {
        m_originCombo->blockSignals(true);
        m_originCombo->setCurrentIndex(index);
        m_originCombo->blockSignals(false);
    }
    if (m_originActionGroup) {
        const auto actions = m_originActionGroup->actions();
        if (index >= 0 && index < actions.size())
            actions[index]->setChecked(true);
    }
}

void MainWindow::onZoomChanged(int /*index*/)
{
    QString text = m_zoomCombo->currentText().remove('%').trimmed();
    if (text.compare(QStringLiteral("适应"), Qt::CaseInsensitive) == 0) {
        m_imageViewer->zoomFit();
        return;
    }
    bool ok;
    int pct = text.toInt(&ok);
    if (ok && pct > 0) {
        m_imageViewer->setZoom(pct / 100.0);
    }
}

void MainWindow::onMousePositionChanged(const QPoint &imagePos)
{
    if (imagePos.x() >= 0 && imagePos.y() >= 0)
        m_pixelPosLabel->setText(QStringLiteral("X: %1  Y: %2")
                                     .arg(imagePos.x()).arg(imagePos.y()));
    else
        m_pixelPosLabel->clear();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, QStringLiteral("关于 YUV 查看器"),
        QStringLiteral("<h3>YUV 查看器 v1.0</h3>"
                       "<p>一个用于查看 YUV 原始图像的工具。</p>"
                       "<p>支持 YV12 / NV12 / NV21 格式，"
                       "提供像素标尺、矩形标注、图片旋转和坐标原点选择功能。</p>"
                       "<p>作者：lindenyu</p>"));
}

// --------------- Drag & Drop ---------------

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString path = urls.first().toLocalFile();
    if (path.isEmpty())
        return;

    int fmtIndex, w = 0, h = 0;
    if (OpenYuvDialog::detectFromFilename(path, fmtIndex, w, h) && w > 0 && h > 0) {
        openYuvFile(path, w, h, fmtIndex);
    } else {
        QMessageBox::information(this, QStringLiteral("无法自动检测"),
            QStringLiteral("无法从文件名自动检测图片参数，请使用 文件 > 打开 手动加载。\n\n文件: %1").arg(path));
    }
}

// --------------- Recent Files ---------------

void MainWindow::addRecentFile(const QString &path)
{
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > 8)
        m_recentFiles.removeLast();

    QSettings settings("YuvViewer", "YuvViewer");
    settings.setValue("recentFiles", m_recentFiles);
    updateRecentMenu();
}

void MainWindow::updateRecentMenu()
{
    m_recentMenu->clear();
    if (m_recentFiles.isEmpty()) {
        m_recentMenu->addAction(QStringLiteral("（无最近文件）"))->setEnabled(false);
        return;
    }
    for (const QString &path : m_recentFiles) {
        auto *action = m_recentMenu->addAction(path);
        connect(action, &QAction::triggered, this, [this, path]() {
            onFileOpenRecent(path);
        });
    }
    m_recentMenu->addSeparator();
    m_recentMenu->addAction(QStringLiteral("清除历史"), this, [this]() {
        m_recentFiles.clear();
        QSettings settings("YuvViewer", "YuvViewer");
        settings.remove("recentFiles");
        updateRecentMenu();
    });
}

void MainWindow::onFileOpenRecent(const QString &path)
{
    int fmtIndex, w = 0, h = 0;
    if (OpenYuvDialog::detectFromFilename(path, fmtIndex, w, h) && w > 0 && h > 0) {
        openYuvFile(path, w, h, fmtIndex);
    } else {
        OpenYuvDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted)
            return;
        openYuvFile(dlg.filePath(), dlg.imageWidth(), dlg.imageHeight(),
                    static_cast<int>(dlg.yuvFormat()));
    }
}
