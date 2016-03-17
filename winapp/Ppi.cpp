//
// Created by sergey on 10.03.16.
//
#include <QDebug>
#include <QRectF>
#include <QGraphicsPathItem>
#include <QApplication>
#include <QDesktopWidget>

#include "Ppi.h"


Ppi::Ppi(QWidget *parent) :
        QGraphicsView(parent), MAX_LENGTH(PULSE_LENGTH * PPI_RADIUS),
        desktopScale(1.0), measurementUnit(metric) {
    qreal distance = MINIMAL_ZOOM_NM;
    for (int i = 1; distance <= MAX_LENGTH; ++i) {
        zoomScaleMaxLengthNm.append(distance);
        distance += ZOOM_STEP_NM;
    }
    distance = MINIMAL_ZOOM_KM;
    for (int i = 1; distance <= MAX_LENGTH; ++i) {
        zoomScaleMaxLengthKm.append(distance);
        distance += ZOOM_STEP_KM;
    }
    zoomScale = zoomScaleMaxLengthKm.size() - 1;
    zoomScaleFactor = qreal(MAX_LENGTH) / qreal(zoomScaleMaxLengthKm[zoomScale]);
    scene_initialize();
    setPpiBackground(Qt::black);
    repaintMesh();
}

void Ppi::scene_initialize() {
    QRectF sceneRect(
            -qreal(SCENE_WIDTH) / 2.0, -qreal(SCENE_HEIGHT) / 2.0,
            SCENE_WIDTH, SCENE_HEIGHT
    );
    ppiScene = new QGraphicsScene(sceneRect);
    meshParent = new QGraphicsRectItem();
    meshParent->setFlags(
            meshParent->flags() | \
            QGraphicsItem::ItemDoesntPropagateOpacityToChildren | \
            QGraphicsItem::ItemHasNoContents
    );
    meshParent->setOpacity(1.0);
    meshDenseParent = new QGraphicsRectItem();
    meshDenseParent->setOpacity(1.0);
    meshDenseParent->setZValue(20.0);
    meshDenseParent->setParentItem(meshParent);
    meshTextParent = new QGraphicsRectItem();
    meshTextParent->setFlags(
            meshTextParent->flags() | \
            QGraphicsItem::ItemDoesntPropagateOpacityToChildren | \
            QGraphicsItem::ItemHasNoContents
    );
    meshTextParent->setOpacity(1.0);
    meshTextParent->setZValue(30.0);
    meshTextParent->setParentItem(meshParent);

    ppiScene->addItem(meshParent);

    QRect geom = qApp->desktop()->availableGeometry(this);
    desktopScale = qreal(geom.height() + geom.y()) / sceneRect.height();
    setRenderHints(renderHints() | QPainter::Antialiasing);
    setScene(ppiScene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(desktopScale, desktopScale);
}

void Ppi::changePpiScale(quint8 newZoomScale) {
    zoomScale = newZoomScale;
    if (getMeasurementUnit() == metric) {
        zoomScaleFactor = qreal(MAX_LENGTH) / qreal(zoomScaleMaxLengthKm[zoomScale]);
    } else {
        zoomScaleFactor = qreal(MAX_LENGTH) / qreal(zoomScaleMaxLengthNm[zoomScale]);
    }
    repaintMesh();
}

void Ppi::changeMeasurementUnit(MeasurementUnit newMeasurementUnit) {
    if (newMeasurementUnit == measurementUnit) {
        return;
    }
    measurementUnit = newMeasurementUnit;
}

void Ppi::repaintMesh() {
    foreach(auto meshIter, meshFiber) {
        delete meshIter;
    }
    meshFiber.clear();
    foreach(auto textIter, meshText) {
        delete textIter;
    }
    meshText.clear();

    qreal scale_step = (getMeasurementUnit()==metric)?ZOOM_STEP_KM:ZOOM_STEP_NM;
    qreal dist = (getMeasurementUnit()==metric)?ZOOM_STEP_KM:ZOOM_STEP_NM;
    qreal min_scale_step = (getMeasurementUnit()==metric)?ZOOM_DENSE_STEP_KM:ZOOM_DENSE_STEP_NM;
    qreal min_scale_dist = (getMeasurementUnit()==metric)?ZOOM_DENSE_STEP_KM:ZOOM_DENSE_STEP_NM;
    const qreal stopZoomScale = (getMeasurementUnit()==metric)?
                                zoomScaleMaxLengthKm[zoomScale]:
                                zoomScaleMaxLengthNm[zoomScale];

    for (quint32 i = 1; dist <= stopZoomScale; ++i) {
        quint32 radius = dist/PULSE_LENGTH;
        qreal penWidth;
        if (i % 2 == 0) {
            penWidth = PEN_WIDTH_THICK;
        } else {
            penWidth = PEN_WIDTH;
        }
        QRectF zoneRect(
                -qreal(radius)*zoomScaleFactor, -qreal(radius)*zoomScaleFactor,
                2 * qreal(radius)*zoomScaleFactor, 2*qreal(radius)*zoomScaleFactor
        );
        QGraphicsEllipseItem *zoneRing = ppiScene->addEllipse(zoneRect, QPen(MESH_COLOR, penWidth, Qt::SolidLine));
        zoneRing->setOpacity(1.0);
        zoneRing->setParentItem(meshParent);
        zoneRing->setZValue(10.0);
        meshFiber.append(zoneRing);

        for (quint32 j = 0; min_scale_dist<dist; ++j) {
            quint32 min_radius = min_scale_dist / PULSE_LENGTH;
            QRectF minZoneRect(
                    -qreal(min_radius)*zoomScaleFactor, -qreal(min_radius)*zoomScaleFactor,
                    2 * qreal(min_radius)*zoomScaleFactor, 2 * qreal(min_radius)*zoomScaleFactor
            );
            QGraphicsEllipseItem *minZoneRing = ppiScene->addEllipse(
                    minZoneRect, QPen(MESH_COLOR, PEN_WIDTH_THIN, Qt::DotLine)
            );
            minZoneRing->setOpacity(1.0);
            minZoneRing->setParentItem(meshDenseParent);
            minZoneRing->setZValue(10.0);
            meshFiber.append(minZoneRing);
            min_scale_dist += min_scale_step;
        }
        min_scale_dist = dist + min_scale_step;

        qreal gap = (dist==stopZoomScale)?2.0:6.0; // Last text element less shifted

        QGraphicsSimpleTextItem *rightLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/((getMeasurementUnit()==metric)?1000.0:NM*1000.0))
        );
        QFont labelFont = rightLabel->font();
        labelFont.setPointSize(labelFont.pointSize()/desktopScale);
        rightLabel->setFont(labelFont);
        rightLabel->setBrush(MESH_TEXT);
        QRectF bRect = rightLabel->boundingRect();
        rightLabel->setPos(qreal(radius)*zoomScaleFactor-bRect.width()-bRect.height()/gap, 0);
        rightLabel->setParentItem(meshTextParent);
        meshText.append(rightLabel);

        QGraphicsSimpleTextItem *leftLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/((getMeasurementUnit()==metric)?1000.0:NM*1000.0))
        );
        leftLabel->setFont(labelFont);
        leftLabel->setBrush(MESH_TEXT);
        leftLabel->setPos(-qreal(radius)*zoomScaleFactor+bRect.height()/gap, -bRect.height());
        leftLabel->setParentItem(meshTextParent);
        meshText.append(leftLabel);
        dist += scale_step;
    }

    QGraphicsSimpleTextItem textSample(QString("%1").arg(360)); // for font metric only
    QFont labelFont = textSample.font();
    labelFont.setPointSize(labelFont.pointSize()/desktopScale);
    textSample.setFont(labelFont);
    QRectF br = textSample.boundingRect(); // Rescaled text border

    QGraphicsLineItem *  azimuthalLine;
    QGraphicsSimpleTextItem *  azimuthalText;
    QGraphicsPathItem *  azimuthalTextBg;
    qreal radialFrom;
    for (quint32 i = 0; i < 36; ++i) {
        quint16 azimuth = i * 10;
        qreal penWidth;
        if (i % 9 != 0) {
            radialFrom =
                    zoomScaleFactor * ((getMeasurementUnit() == metric) ? ZOOM_STEP_KM : ZOOM_STEP_NM) /
                    (4.0 * PULSE_LENGTH);
        } else {
            radialFrom = 0.0;
        }
        if (i % 9 == 0) {
            penWidth = PEN_WIDTH_THICK;
        } else if (i % 3 == 0) {
            penWidth = PEN_WIDTH;
        } else {
            penWidth = PEN_WIDTH_THIN;
        }
        azimuthalLine = new QGraphicsLineItem(
                sceneRect().center().x(), sceneRect().center().y() + radialFrom,
                sceneRect().center().x(), PPI_RADIUS);
        azimuthalLine->setPen(QPen(MESH_COLOR, penWidth, Qt::SolidLine));
        azimuthalLine->setTransform(
                azimuthalLine->transform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                rotate(azimuth).\
                translate(-sceneRect().center().x(), -sceneRect().center().y())
        );
        azimuthalLine->setOpacity(1.0);
        azimuthalLine->setParentItem(meshParent);
        azimuthalLine->setZValue(10.0); // !!! - set value less than for all meshTextParent childs
        meshFiber.append(azimuthalLine);
        azimuthalTextBg = new QGraphicsPathItem();
        azimuthalTextBg->setOpacity(1.0);
        azimuthalTextBg->setBrush(scene()->backgroundBrush());
        azimuthalTextBg->setPen(QPen(MESH_COLOR, PEN_WIDTH_THIN, Qt::SolidLine));
        azimuthalTextBg->setParentItem(meshTextParent);

        QPainterPath roundRect;
        roundRect.addRoundedRect(br - QMargins(-4, 2, -4, 2), 24, 24);
        azimuthalTextBg->setPath(roundRect);

        azimuthalText = new QGraphicsSimpleTextItem(QString("%1").arg(azimuth));
        azimuthalText->setParentItem(azimuthalTextBg);
        azimuthalText->setFont(labelFont);
        azimuthalText->setBrush(MESH_TEXT);
        azimuthalText->setPos((br.width()-azimuthalText->boundingRect().width())/2, 0);

        if (azimuth >= 270 || azimuth <= 90) {
            azimuthalTextBg->setTransform(
                    azimuthalText->transform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                rotate(azimuth).\
                translate(sceneRect().center().x() - br.width() / 2.0,
                          sceneRect().center().y() - PPI_RADIUS - br.height() / 2)
            );
        } else {
            azimuthalTextBg->setTransform(
                    azimuthalText->transform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                rotate(180.+azimuth).\
                translate(sceneRect().center().x() - br.width() / 2.0,
                          sceneRect().center().y() + PPI_RADIUS - br.height() / 2)
            );
        }
        meshText.append(azimuthalTextBg);
    }
}

const QVector<qreal> &Ppi::getZoomScales() const {
    return (measurementUnit==metric)?zoomScaleMaxLengthKm:zoomScaleMaxLengthNm;
}

void Ppi::setPpiBackground(const QBrush &brush) {
    ppiScene->setBackgroundBrush(brush);
}

void Ppi::drawMesh(bool draw) {
    meshParent->setVisible(draw);
}

void Ppi::drawDenseMesh(bool draw) {
    meshDenseParent->setVisible(draw);
}

void Ppi::drawMeshText(bool draw) {
    meshTextParent->setVisible(draw);
}
