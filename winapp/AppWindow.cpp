//
// Created by sergey on 03.03.16.
//

#include "AppWindow.h"

AppWindow::AppWindow(QWidget * parent):
    QMainWindow(parent), isFullScreenMode(false)
{
    CreateWidgets();
    CreateLayouts();
    CreateConnections();
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint);
    setWindowTitle(tr("PPI"));
}

void AppWindow::CreateWidgets()
{
    topWidget = new TopWidget(this);
    setCentralWidget(topWidget);
}

void AppWindow::CreateLayouts()
{

}

void AppWindow::CreateConnections()
{

}

void AppWindow::mouseDoubleClickEvent(QMouseEvent *)
{
    if (isFullScreenMode) {
        showMaximized();
        isFullScreenMode = false;
    } else {
        showFullScreen();
        isFullScreenMode = true;
    }
}

AppWindow::~AppWindow()
{

}
