#include <QApplication>

#include "AppWindow.h"

Q_DECLARE_METATYPE(Ppi::PpiZoomScale)

int main(int argc, char *argv[]) {
    qRegisterMetaType<Ppi::PpiZoomScale>("PpiZoomScale");

    QApplication a(argc, argv);
    AppWindow appWindow;
    appWindow.showMaximized();
    return a.exec();
}
