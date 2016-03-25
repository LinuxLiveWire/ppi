#include <QApplication>

#include "AppWindow.h"
#include "ppidefs.h"

Q_DECLARE_METATYPE(MeasurementUnit)
Q_DECLARE_METATYPE(ScanIndicatorType)

int main(int argc, char *argv[]) {
    qRegisterMetaType<MeasurementUnit>("MeasurementUnit");
    qRegisterMetaType<ScanIndicatorType>("ScanIndicatorType");
    QApplication a(argc, argv);
    AppWindow appWindow;
    appWindow.showMaximized();
    return a.exec();
}
