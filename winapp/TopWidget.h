//
// Created by sergey on 04.03.16.
//

#ifndef PPI_TOPWIDGET_H
#define PPI_TOPWIDGET_H

#include <QWidget>

#include "Ppi.h"
#include "PpiControl.h"


class TopWidget: public QWidget {
public:
    TopWidget(QWidget* parent = nullptr);
private:
    void CreateWidgets();
    void CreateLayouts();
    void CreateConnections();
private:
    Ppi *   ppi;
    PpiControl *   ppiControl;
};

#endif //PPI_TOPWIDGET_H
