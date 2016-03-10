#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>

#include "TopWidget.h"

class AppWindow : public QMainWindow
{
Q_OBJECT

public:
    AppWindow(QWidget * parent = 0);
    ~AppWindow();
    void mouseDoubleClickEvent(QMouseEvent *);
private:
    void CreateWidgets();
    void CreateLayouts();
    void CreateConnections();
private:
    TopWidget * topWidget;
    bool isFullScreenMode;
};

#endif // APPWINDOW_H
