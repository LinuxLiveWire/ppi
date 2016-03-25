//
// Created by sergey on 14.03.16.
//

#ifndef PPI_PPICONTROL_H
#define PPI_PPICONTROL_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QTimer>

#include "Ppi.h"


class PpiControl: public QWidget {
    Q_OBJECT
signals:
    void zoomScaleChanged(quint8);
    void measurementUnitChanged(MeasurementUnit);
    void rotateScanIndicator(qreal);
public:
    PpiControl(Ppi*, QWidget* parent = 0);
    void setZoomScale(quint8);
    void setMeasurementUnit(MeasurementUnit);
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
    QCheckBox *  drawMesh, * drawDenseMesh, * drawMeshText;
    QCheckBox *  zoomView;
    QRadioButton *  lineIndicator, * dotsIndicator, * pointerIndicator;
    QTimer *  rotationTimer;
    quint64 counter;
};

#endif //PPI_PPICONTROL_H
