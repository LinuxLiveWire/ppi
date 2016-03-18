//
// Created by sergey on 04.03.16.
//

#include "TopWidget.h"
#include <QSplitter>
#include <QHBoxLayout>

TopWidget::TopWidget(QWidget* parent):
        QWidget(parent)
{
    CreateWidgets();
    CreateLayouts();
    CreateConnections();
}

void TopWidget::CreateWidgets()
{
    ppi = new Ppi(this);
    ppiControl = new PpiControl(ppi, this);
}

void TopWidget::CreateLayouts()
{
    int index;
    QSplitter *  splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(ppi);
    splitter->addWidget(ppiControl);
    QHBoxLayout *  topLayout = new QHBoxLayout;
    topLayout->addWidget(splitter);
    index = splitter->indexOf(ppi);
    splitter->setCollapsible(index, false);
    splitter->setStretchFactor(index, 2);
    index = splitter->indexOf(ppiControl);
    splitter->setCollapsible(index, true);
    splitter->setStretchFactor(index, 1);
    ppiControl->setMaximumWidth(160);
    ppiControl->setMinimumWidth(80);
    topLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(topLayout);
}

void TopWidget::CreateConnections()
{
    connect(ppiControl, SIGNAL(zoomScaleChanged(quint8)),
            ppi, SLOT(changePpiScale(quint8))
    );
    connect(ppiControl, SIGNAL(measurementUnitChanged(Ppi::MeasurementUnit)),
            ppi, SLOT(changeMeasurementUnit(Ppi::MeasurementUnit))
    );
    connect(ppiControl, SIGNAL(rotateScanIndicator(qreal)), ppi, SLOT(scanIndicatorRotate(qreal)));
}
