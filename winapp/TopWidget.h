//
// Created by sergey on 04.03.16.
//

#include <QWidget>

#ifndef PPI_TOPWIDGET_H
#define PPI_TOPWIDGET_H

class TopWidget: public QWidget {
public:
    TopWidget(QWidget* parent = nullptr);
private:
    void CreateWidgets();
    void CreateLayouts();
    void CreateConnections();
};

#endif //PPI_TOPWIDGET_H
