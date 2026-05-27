#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("YUV Viewer");
    app.setApplicationVersion("1.0.0");

    MainWindow window;
    window.resize(1280, 800);
    window.show();

    return app.exec();
}
