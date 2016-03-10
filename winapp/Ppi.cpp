//
// Created by sergey on 10.03.16.
//
#include <QRectF>
#include <QGraphicsRectItem>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

#include "Ppi.h"

#define SCENE_SIDE      4000
#define SCENE_WIDTH     SCENE_SIDE
#define SCENE_HEIGHT    SCENE_WIDTH

#define PEN_WIDTH       4.0

Ppi::Ppi(QWidget* parent):
        QGraphicsView(parent), desktopScale(1.0)
{
    scene_initialize();
}

void Ppi::scene_initialize()
{
    QRectF sceneRect(-SCENE_WIDTH/2.0, -SCENE_HEIGHT/2.0, SCENE_WIDTH, SCENE_HEIGHT);
    ppiScene = new QGraphicsScene(sceneRect);
    ppiScene->setBackgroundBrush(Qt::darkBlue);
    QGraphicsRectItem *  rect = ppiScene->addRect(
        -SCENE_WIDTH/2.0+2.0, -SCENE_HEIGHT/2.0+2.0,
        SCENE_WIDTH-4.0, SCENE_HEIGHT-4.0
    );
    rect->setPen(QPen(Qt::yellow, PEN_WIDTH, Qt::SolidLine));
    QGraphicsLineItem *  line1 = ppiScene->addLine(-SCENE_WIDTH/2.0+2.0, -SCENE_HEIGHT/2.0+2.0,
                                                   SCENE_WIDTH/2.0-2.0, SCENE_HEIGHT/2.0-2.0);
    line1->setPen(QPen(Qt::yellow, PEN_WIDTH, Qt::SolidLine));
    QGraphicsEllipseItem *  outerRing = ppiScene->addEllipse(sceneRect, QPen(Qt::yellow, PEN_WIDTH, Qt::SolidLine));
    QRect geom = qApp->desktop()->availableGeometry(this);
    desktopScale = qreal(geom.height()+geom.y())/sceneRect.height();
    setRenderHints(renderHints()|QPainter::Antialiasing);
    setScene(ppiScene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(desktopScale, desktopScale);
}
