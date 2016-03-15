#include <QApplication>

#include "AppWindow.h"

Q_DECLARE_METATYPE(Ppi::MeasurementUnit)

int main(int argc, char *argv[]) {
    qRegisterMetaType<Ppi::MeasurementUnit>("MeasurementUnit");

    QApplication a(argc, argv);
    AppWindow appWindow;
    appWindow.showMaximized();
    return a.exec();
}
