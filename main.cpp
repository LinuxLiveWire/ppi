#include <QApplication>

#include "AppWindow.h"

Q_DECLARE_METATYPE(Ppi::MeasurementUnit)
Q_DECLARE_METATYPE(Ppi::ScanIndicatorType)

int main(int argc, char *argv[]) {
    qRegisterMetaType<Ppi::MeasurementUnit>("MeasurementUnit");
    qRegisterMetaType<Ppi::ScanIndicatorType>("ScanIndicatorType");
    QApplication a(argc, argv);
    AppWindow appWindow;
    appWindow.showMaximized();
    return a.exec();
}
