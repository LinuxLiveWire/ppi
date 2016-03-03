#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QMainWindow>

class AppWindow : public QMainWindow
{
Q_OBJECT

public:
    AppWindow(QWidget * parent = 0);
    ~AppWindow();
};

#endif // APPWINDOW_H
