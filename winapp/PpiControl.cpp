//
// Created by sergey on 14.03.16.
//
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDebug>

#include "Ppi.h"
#include "PpiControl.h"

PpiControl::PpiControl(Ppi *p_ppi, QWidget *parent):
        QWidget(parent), ppi(p_ppi)
{
    createWidgets();
    createLayouts();
    createConnections();
}

void PpiControl::createWidgets()
{
    Ppi::MeasurementUnit unit = ppi->getMeasurementUnit();
    const QVector<qreal> &zoomScale = ppi->getZoomScales();
    quint8 currentZoomScale = ppi->getZoomScale();
    bZoomScale = new QComboBox(this);
    if (unit==Ppi::metric) {
        for (int i=0; i<zoomScale.size(); ++i) {
            bZoomScale->addItem(
                    QString("x%1").arg((MINIMAL_ZOOM_KM+i*ZOOM_STEP_KM)/1000.0),
                    QVariant::fromValue((quint8)i)
            );
        }
    } else {
        for(int i=0; i<zoomScale.size(); ++i) {
            bZoomScale->addItem(
                    QString("x%1").arg((KM2NM(MINIMAL_ZOOM_NM)+i*KM2NM(ZOOM_STEP_NM))/1000.0),
                    QVariant::fromValue((quint8)i)
            );
        }
    }
    bZoomScale->setCurrentIndex(currentZoomScale);
    bUnits = new QComboBox(this);
    bUnits->addItem("Metric", QVariant::fromValue((int)Ppi::metric));
    bUnits->addItem("Imperial", QVariant::fromValue((int)Ppi::imperial));
    bUnits->setCurrentIndex(bUnits->findData(QVariant::fromValue((int)unit)));
    drawMesh = new QCheckBox("Mesh:", this);
    drawMesh->setChecked(true);
    drawDenseMesh = new QCheckBox("Dense mesh:", this);
    drawDenseMesh->setChecked(true);
    drawMeshText = new QCheckBox("Label:", this);
    drawMeshText->setChecked(true);
}

void PpiControl::createLayouts()
{
    QVBoxLayout *  widgetLayout = new QVBoxLayout;
    QGridLayout *  controlLayout = new QGridLayout;

    controlLayout->addWidget( new QLabel("Scale:"), 1, 1, Qt::AlignRight|Qt::AlignVCenter);
    controlLayout->addWidget( bZoomScale, 1, 2, Qt::AlignLeft|Qt::AlignVCenter);
    controlLayout->addWidget( new QLabel("Unit:"), 2, 1, Qt::AlignRight|Qt::AlignVCenter);
    controlLayout->addWidget( bUnits, 2, 2, Qt::AlignLeft|Qt::AlignVCenter);
    controlLayout->setColumnStretch(0, 1);
    controlLayout->setColumnStretch(3, 1);

    QVBoxLayout *  meshControlLayout = new QVBoxLayout;
    meshControlLayout->addWidget(drawMesh);
    meshControlLayout->addWidget(drawDenseMesh);
    meshControlLayout->addWidget(drawMeshText);
    QGroupBox *  meshGroup = new QGroupBox("Mesh", this);
    meshGroup->setLayout(meshControlLayout);

    widgetLayout->addLayout(controlLayout);
    widgetLayout->addWidget(meshGroup);
    widgetLayout->addStretch(1);
    widgetLayout->setContentsMargins(0, 11, 0, 11);
    setLayout(widgetLayout);
}

void PpiControl::createConnections()
{
    const QVector<qreal> &zoomScale = ppi->getZoomScales();
    connect(bZoomScale, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [=](int index){
        emit zoomScaleChanged((quint8)index);
    });
    connect(bUnits, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [=](int index){
        emit measurementUnitChanged((Ppi::MeasurementUnit)bUnits->itemData(index).toInt());
        onMetricChanged();
    });
    connect(drawMesh, SIGNAL(clicked(bool)), ppi, SLOT(drawMesh(bool)));
    connect(drawDenseMesh, SIGNAL(clicked(bool)), ppi, SLOT(drawDenseMesh(bool)));
    connect(drawMesh, SIGNAL(clicked(bool)), drawMeshText, SLOT(setEnabled(bool)));
    connect(drawMesh, SIGNAL(clicked(bool)), drawDenseMesh, SLOT(setEnabled(bool)));
    connect(drawMeshText, SIGNAL(clicked(bool)), ppi, SLOT(drawMeshText(bool)));
}

void PpiControl::onMetricChanged()
{
    Ppi::MeasurementUnit unit = ppi->getMeasurementUnit();
    const QVector<qreal> &zoomScale = ppi->getZoomScales();
    int scaleIndex = bZoomScale->currentData().toInt();
    bZoomScale->clear();
    if (unit==Ppi::metric) {
        for (int i=0; i<zoomScale.size(); ++i) {
            bZoomScale->addItem(
                    QString("x%1").arg((MINIMAL_ZOOM_KM+i*ZOOM_STEP_KM)/1000.0),
                    QVariant::fromValue((quint8)i)
            );
        }
        bZoomScale->setCurrentIndex(scaleIndex*2);
    } else {
        for(int i=0; i<zoomScale.size(); ++i) {
            bZoomScale->addItem(
                    QString("x%1").arg((KM2NM(MINIMAL_ZOOM_NM)+i*KM2NM(ZOOM_STEP_NM))/1000.0),
                    QVariant::fromValue((quint8)i)
            );
        }
        bZoomScale->setCurrentIndex(scaleIndex/2);
    }
    emit zoomScaleChanged((quint8)bZoomScale->currentIndex());
}

void PpiControl::setZoomScale(quint8 item)
{
    int index = bZoomScale->findData(QVariant::fromValue(item));
    if (index>=0) {
        bZoomScale->setCurrentIndex(index);
    }
}

void PpiControl::setMeasurementUnit(Ppi::MeasurementUnit newUnit)
{
    int index = bUnits->findData(QVariant::fromValue((int)newUnit));
    if (index>=0) {
        bUnits->setCurrentIndex(index);
        onMetricChanged();
    }
}
