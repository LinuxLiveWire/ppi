//
// Created by sergey on 03.03.16.
//

#include "AppWindow.h"

AppWindow::AppWindow(QWidget * parent):
    QMainWindow(parent)
{
    CreateWidgets();
    CreateLayouts();
    CreateConnections();
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

AppWindow::~AppWindow()
{

}
