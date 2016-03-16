//
// Created by sergey on 10.03.16.
//
#include <QDebug>
#include <QRectF>
#include <QGraphicsRectItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>

#include "Ppi.h"

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

Ppi::Ppi(QWidget* parent):
        QGraphicsView(parent),
        desktopScale(1.0), MAX_LENGTH(PULSE_LENGTH*PPI_RADIUS),
        measurementUnit(metric)
{
    qreal distance = MINIMAL_ZOOM_NM;
    for(int i=1; distance<=MAX_LENGTH; ++i){
        zoomScaleMaxLengthNm.append(distance);
        distance += ZOOM_STEP_NM;
    }
    distance = MINIMAL_ZOOM_KM;
    for(int i=1; distance<=MAX_LENGTH; ++i){
        zoomScaleMaxLengthKm.append(distance);
        distance += ZOOM_STEP_KM;
    }
    zoomScale = zoomScaleMaxLengthKm.size()-1;
    zoomScaleFactor = qreal(MAX_LENGTH)/qreal(zoomScaleMaxLengthKm[zoomScale]);
    scene_initialize();
    setPpiBackground(Qt::black);
    repaintMesh();
}

void Ppi::scene_initialize()
{
    QRectF sceneRect(-qreal(SCENE_WIDTH)/2.0, -qreal(SCENE_HEIGHT)/2.0, SCENE_WIDTH, SCENE_HEIGHT);
    ppiScene = new QGraphicsScene(sceneRect);
    meshParent = new  QGraphicsRectItem();
    meshTextParent = new  QGraphicsRectItem(meshParent);
    ppiScene->addItem(meshParent);
    QRect geom = qApp->desktop()->availableGeometry(this);
    desktopScale = qreal(geom.height()+geom.y())/sceneRect.height();
    setRenderHints(renderHints()|QPainter::Antialiasing);
    setScene(ppiScene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(desktopScale, desktopScale);
}

void Ppi::changePpiScale(quint8 newZoomScale)
{
    zoomScale=newZoomScale;
    if (getMeasurementUnit()==metric) {
        zoomScaleFactor = qreal(MAX_LENGTH)/qreal(zoomScaleMaxLengthKm[zoomScale]);
    } else {
        zoomScaleFactor = qreal(MAX_LENGTH)/qreal(zoomScaleMaxLengthNm[zoomScale]);
    }
    repaintMesh();
}

void Ppi::changeMeasurementUnit(MeasurementUnit newMeasurementUnit)
{
    if (newMeasurementUnit==measurementUnit) {
        return;
    }
    measurementUnit=newMeasurementUnit;
}

void Ppi::repaintMesh()
{
    foreach(auto meshIter, radialMesh) {
        delete meshIter;
    }
    radialMesh.clear();
    foreach(auto textIter, radialText) {
        delete textIter;
    }
    radialText.clear();

    qreal zoom_step = getMeasurementUnit()==metric?ZOOM_STEP_KM:ZOOM_STEP_NM;
    qreal dist = getMeasurementUnit()==metric?ZOOM_STEP_KM:ZOOM_STEP_NM;
    const qreal stopZoomScale = getMeasurementUnit()==metric?
                                zoomScaleMaxLengthKm[zoomScale]:
                                zoomScaleMaxLengthNm[zoomScale];

    for(quint32 i=1; dist<=stopZoomScale; ++i) {
        quint32 radius = dist/PULSE_LENGTH;
        qreal penWidth;
        if (i%2==0) {
            penWidth = PEN_WIDTH_THICK;
        } else {
            penWidth = PEN_WIDTH;
        }
        QRectF zoneRect(
                -qreal(radius)*zoomScaleFactor, -qreal(radius)*zoomScaleFactor,
                2*qreal(radius)*zoomScaleFactor, 2*qreal(radius)*zoomScaleFactor
        );
        QGraphicsEllipseItem *  zoneRing = ppiScene->addEllipse(zoneRect, QPen(MESH_COLOR, penWidth, Qt::SolidLine));
        zoneRing->setParentItem(meshParent);
        radialMesh.append(zoneRing);
        QGraphicsSimpleTextItem *  rightLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/(getMeasurementUnit()==metric?1000.0:NM*1000.0))
        );
        QFont labelFont = rightLabel->font();
        labelFont.setPointSize(labelFont.pointSize()/desktopScale);
        rightLabel->setFont(labelFont);
        rightLabel->setBrush(MESH_TEXT);
        QRectF bRect = rightLabel->boundingRect();
        rightLabel->setPos( qreal(radius)*zoomScaleFactor-bRect.width(), 0);
        rightLabel->setParentItem(meshTextParent);
        radialText.append(rightLabel);
        QGraphicsSimpleTextItem *  leftLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/(getMeasurementUnit()==metric?1000.0:NM*1000.0))
        );
        leftLabel->setFont(labelFont);
        leftLabel->setBrush(MESH_TEXT);
        leftLabel->setPos( -qreal(radius)*zoomScaleFactor, -bRect.height());
        leftLabel->setParentItem(meshTextParent);
        radialText.append(leftLabel);
        dist += zoom_step;
    }
    QGraphicsLineItem *  azimutalLine;
    QGraphicsSimpleTextItem *  azimutalText;
    qreal radialFrom;
    for(quint32 i=0; i<36; ++i) {
        qreal penWidth;
        if (i%9!=0) {
            radialFrom = zoomScaleFactor*(getMeasurementUnit()==metric?ZOOM_STEP_KM:ZOOM_STEP_NM)/PULSE_LENGTH;
        } else {
            radialFrom = 0.0;
        }
        if (i%9==0) {
            penWidth = PEN_WIDTH_THICK;
        } else if (i%3==0) {
            penWidth = PEN_WIDTH;
        } else {
            penWidth = PEN_WIDTH_THIN;
        }
        azimutalLine = new QGraphicsLineItem(
                sceneRect().center().x(), sceneRect().center().y()+radialFrom,
                sceneRect().center().x(), PPI_RADIUS, meshParent );
        azimutalLine->setPen( QPen(MESH_COLOR, penWidth, Qt::SolidLine) );
        azimutalLine->setTransform(
                azimutalLine->transform().\
                translate( sceneRect().center().x(),sceneRect().center().y() ).\
                rotate( 10.0 * i ).\
                translate( -sceneRect().center().x(), -sceneRect().center().y() )
        );
        radialMesh.append(azimutalLine);
        azimutalText = new QGraphicsSimpleTextItem( QString("%1").arg( i*10 ), meshTextParent );
        QFont labelFont = azimutalText->font();
        labelFont.setPointSize(labelFont.pointSize()/desktopScale);
        azimutalText->setFont(labelFont);
        azimutalText->setBrush(MESH_TEXT);
        QRectF br = azimutalText->boundingRect();
        qreal dX = 0., dY= 0.;
        if (i*10<90. && i*10>0.) {
            dX=0; dY=-br.height();
        } else if (i*10<180. && i*10>90.) {
            dX=0.; dY=0.;
        } else if (i*10<270. && i*10>180.) {
            dX=-br.width(); dY=0.;
        } else if (i*10<360. && i*10>270.) {
            dX=-br.width(); dY=-br.height();
        } else if (i*10==0.0) {
            dX=-br.width()/2.0; dY=-br.height();
        } else if (i*10==90.0) {
            dX=0; dY=-br.height()/2.0;
        } else if (i*10==180.0) {
            dX=-br.width()/2.0; dY=br.height();
        } else if (i*10==270.0) {
            dX=-br.width(); dY=-br.height()/2.0;
        }
        azimutalText->setPos(
                sceneRect().center().x()+dX+qSin(qDegreesToRadians(180.0-10.0*i))*PPI_RADIUS,
                sceneRect().center().y()+dY+qCos(qDegreesToRadians(180-10.0*i))*PPI_RADIUS
        );
        radialText.append(azimutalText);
    }
}

const QVector<qreal>& Ppi::getZoomScales() const
{
    return (measurementUnit==metric)?zoomScaleMaxLengthKm:zoomScaleMaxLengthNm;
}

void Ppi::setPpiBackground(const QBrush& brush)
{
    ppiScene->setBackgroundBrush(brush);
}

void Ppi::drawMesh(bool draw)
{
    meshParent->setVisible(draw);
}

void Ppi::drawMeshText(bool draw)
{
    meshTextParent->setVisible(draw);
}
