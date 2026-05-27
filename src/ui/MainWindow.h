#pragma once

#include <QMainWindow>

class QScrollArea;
class QLabel;
class QDockWidget;
class QComboBox;
class QAction;
class QActionGroup;
class ImageViewer;
class CoordinateInputPanel;
template <typename T> class QFutureWatcher;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onFileOpen();
    void onCloseImage();
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onResetZoom();
    void onZoomChanged(int index);
    void onMousePositionChanged(const QPoint &imagePos);
    void onRotateLeft();
    void onRotateRight();
    void onResetRotation();
    void onOriginChanged(int index);
    void onAbout();
    void onFileOpenRecent(const QString &path);
    void addRecentFile(const QString &path);
    void updateRecentMenu();

private:
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDockPanel();
    void updateActionStates(bool hasImage);
    bool openYuvFile(const QString &filePath, int width, int height, int formatIndex);

    ImageViewer *m_imageViewer = nullptr;
    CoordinateInputPanel *m_coordPanel = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QLabel *m_imageInfoLabel = nullptr;
    QLabel *m_pixelPosLabel = nullptr;
    QComboBox *m_zoomCombo = nullptr;
    QComboBox *m_originCombo = nullptr;
    QActionGroup *m_originActionGroup = nullptr;
    QDockWidget *m_dockWidget = nullptr;
    QMenu *m_recentMenu = nullptr;
    QStringList m_recentFiles;
    QList<QAction *> m_imageActions;
    QAction *m_closeAction = nullptr;
    QFutureWatcher<QImage> *m_convertWatcher = nullptr;
    int m_openGeneration = 0;
};
