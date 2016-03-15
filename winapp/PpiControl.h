//
// Created by sergey on 14.03.16.
//

#ifndef PPI_PPICONTROL_H
#define PPI_PPICONTROL_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>

#include "Ppi.h"


class PpiControl: public QWidget {
    Q_OBJECT
signals:
    void zoomScaleChanged(quint8);
    void measurementUnitChanged(Ppi::MeasurementUnit);
public:
    PpiControl(Ppi*, QWidget* parent = 0);
    void setZoomScale(quint8);
    void setMeasurementUnit(Ppi::MeasurementUnit);
protected:
    void onMetricChanged();
private:
    void createWidgets();
    void createLayouts();
    void createConnections();
private:
    QComboBox *  bZoomScale;
    QComboBox *  bUnits;
    Ppi *  ppi;
    QCheckBox *  drawMesh, * drawText;
};

#endif //PPI_PPICONTROL_H
