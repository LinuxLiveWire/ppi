#include <QApplication>

#include "AppWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    AppWindow appWindow;
    appWindow.show();
    return a.exec();
}
