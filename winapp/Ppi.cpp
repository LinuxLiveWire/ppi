//
// Created by sergey on 10.03.16.
//
#include <QRectF>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

#include "Ppi.h"


Ppi::Ppi(QWidget* parent):
        QGraphicsView(parent), desktopScale(1.0),
        MAX_LENGTH(PULSE_LENGTH*PPI_RADIUS), zoomScale(xZMax),
        zoomScaleMaxLength(xZMax-xZMin+1)
{
    for(int i = xZMin; i<=xZMax; ++i){
        zoomScaleMaxLength[i] = MINIMAL_ZOOM + i*ZOOM_STEP;
    }
    zoomScaleFactor = qreal(MAX_LENGTH)/qreal(zoomScaleMaxLength[zoomScale]);
    scene_initialize();
}

void Ppi::scene_initialize()
{
    QRectF sceneRect(-qreal(SCENE_WIDTH)/2.0, -qreal(SCENE_HEIGHT)/2.0, SCENE_WIDTH, SCENE_HEIGHT);
    QRectF ppiRect(-qreal(PPI_SIDE)/2.0, -qreal(PPI_SIDE)/2.0, PPI_SIDE, PPI_SIDE);
    ppiScene = new QGraphicsScene(sceneRect);
    ppiScene->setBackgroundBrush(Qt::darkBlue);
    QGraphicsRectItem *  rect = ppiScene->addRect(sceneRect, QPen(Qt::yellow, PEN_WIDTH, Qt::SolidLine));
    QGraphicsLineItem *  line1 = ppiScene->addLine(
            QLine(sceneRect.topLeft().toPoint(), sceneRect.bottomRight().toPoint()),
            QPen(Qt::yellow, PEN_WIDTH, Qt::SolidLine)
    );
    QGraphicsEllipseItem *  outerRing = ppiScene->addEllipse(ppiRect, QPen(Qt::yellow, PEN_WIDTH, Qt::SolidLine));
    //QGraphicsLineItem *  line1 = ppiScene->addLine(-SCENE_WIDTH/2.0+2.0, -SCENE_HEIGHT/2.0+2.0,
    //                                               SCENE_WIDTH/2.0-2.0, SCENE_HEIGHT/2.0-2.0);
    QRect geom = qApp->desktop()->availableGeometry(this);
    desktopScale = qreal(geom.height()+geom.y())/sceneRect.height();
    setRenderHints(renderHints()|QPainter::Antialiasing);
    setScene(ppiScene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(desktopScale, desktopScale);
}

void Ppi::changePpiScale(PpiZoomScale newZoomScale)
{
    if (zoomScale==newZoomScale) {
        return;
    }
    zoomScale=newZoomScale;
    zoomScaleFactor = qreal(MAX_LENGTH)/qreal(zoomScaleMaxLength[zoomScale]);
}

