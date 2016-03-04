#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include "TopWidget.h"

class AppWindow : public QMainWindow
{
Q_OBJECT

public:
    AppWindow(QWidget * parent = 0);
    ~AppWindow();
private:
    void CreateWidgets();
    void CreateLayouts();
    void CreateConnections();
private:
    TopWidget * topWidget;
};

#endif // APPWINDOW_H
