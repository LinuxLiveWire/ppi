//
// Created by sergey on 21.03.16.
//

#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>
#include <QGraphicsItem>
#include <QDebug>
#include "Magnifier.h"

Magnifier::Magnifier(QWidget * parent):
    QGraphicsView(parent), scanIndicatorType(ScanIndicatorType::line),
    MAX_LENGTH(PULSE_LENGTH * PPI_RADIUS), desktopScale(1.0),
    measurementUnit(MeasurementUnit::metric), scanIndicator {nullptr, nullptr, nullptr}
{
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

    setMinimumSize(QSize(100, 100));
    setMaximumSize(QSize(300, 300));
    setRenderHint( QPainter::Antialiasing, true );
    setRenderHint( QPainter::TextAntialiasing, true );
    setRenderHint( QPainter::SmoothPixmapTransform, false );
    setTransformationAnchor( QGraphicsView::AnchorViewCenter );
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setWindowFlags(windowFlags()|Qt::Window|Qt::FramelessWindowHint);
    setStyleSheet("QFrame { border: 1px solid red; }");
    //setFrameStyle(QFrame::NoFrame);
    scene_initialize();
    QRect geom = qApp->desktop()->availableGeometry(this);
    desktopScale = qreal(geom.height() + geom.y()) / sceneRect().height();
    repaintMesh();
    scale(desktopScale, desktopScale);
}

void Magnifier::scene_initialize()
{
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

    scanIndicatorParent = new QGraphicsRectItem();
    scanIndicatorParent->setFlags(
            scanIndicatorParent->flags() | \
            QGraphicsItem::ItemDoesntPropagateOpacityToChildren | \
            QGraphicsItem::ItemHasNoContents
    );
    scanIndicatorParent->setOpacity(1.0);
    scanIndicatorParent->setPos(sceneRect.center());

    const QVector<QPointF> triangle = {
            QPointF(15., -qreal(PPI_RADIUS)), QPointF(sceneRect.center().x(), -qreal(PPI_RADIUS)+45),
            QPointF(-15., -qreal(PPI_RADIUS)), QPointF(15., -qreal(PPI_RADIUS))
    };
    QPolygonF pointerTriangle(triangle);
    scanIndicator.pointer = new QGraphicsPolygonItem(pointerTriangle, scanIndicatorParent);
    scanIndicator.pointer->setBrush(MESH_COLOR);
    scanIndicator.pointer->setPen(QPen(MESH_COLOR, PEN_WIDTH_THICK, Qt::SolidLine));
    scanIndicator.pointer->setVisible(false);
    scanIndicator.line = new QGraphicsLineItem(
            QLineF(
                    sceneRect.center().x(), sceneRect.center().y(),
                    sceneRect.center().x(), -qreal(PPI_RADIUS)
            ),
            scanIndicatorParent
    );
    scanIndicator.line->setPen(QPen(MESH_COLOR, PEN_WIDTH_THICK, Qt::SolidLine));
    scanIndicator.dots = new QGraphicsPathItem(scanIndicatorParent);
    scanIndicator.dots->setBrush(MESH_COLOR);
    scanIndicator.dots->setVisible(false);

    ppiScene->addItem(meshParent);
    ppiScene->addItem(scanIndicatorParent);
    setScene(ppiScene);
}

void Magnifier::repaintMesh() {
    for(auto &meshIter: meshFiber) {
        delete meshIter;
    }
    meshFiber.clear();
    for(auto &textIter: meshText) {
        delete textIter;
    }
    meshText.clear();

    qreal scale_step = (getMeasurementUnit()==MeasurementUnit::metric)?ZOOM_STEP_KM:ZOOM_STEP_NM;
    qreal dist = (getMeasurementUnit()==MeasurementUnit::metric)?ZOOM_STEP_KM:ZOOM_STEP_NM;
    qreal min_scale_step = (getMeasurementUnit()==MeasurementUnit::metric)?ZOOM_DENSE_STEP_KM:ZOOM_DENSE_STEP_NM;
    qreal min_scale_dist = (getMeasurementUnit()==MeasurementUnit::metric)?ZOOM_DENSE_STEP_KM:ZOOM_DENSE_STEP_NM;
    const qreal stopZoomScale = (getMeasurementUnit()==MeasurementUnit::metric)?
                                zoomScaleMaxLengthKm[zoomScale]:
                                zoomScaleMaxLengthNm[zoomScale];

    QPainterPath scanIndicatorPoints;

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

        scanIndicatorPoints.addEllipse(
                QPointF(sceneRect().center().x(), -qreal(radius)*zoomScaleFactor),
                6, 6
        );

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
                QString("%1").arg(dist/((getMeasurementUnit()==MeasurementUnit::metric)?1000.0:NM*1000.0))
        );
        QFont labelFont = rightLabel->font();
        labelFont.setPointSize(labelFont.pointSize()/desktopScale);
        rightLabel->setFont(labelFont);
        rightLabel->setBrush(MESH_TEXT);
        QRectF bRect = rightLabel->boundingRect();
        rightLabel->setPos(qreal(radius)*zoomScaleFactor-bRect.width()-bRect.height()/gap, 0);
        rightLabel->setData(0, QVariant::fromValue(rightLabel->pos().toPoint()));
        rightLabel->setData(1, QVariant::fromValue(bRect.width()));
        rightLabel->setParentItem(meshTextParent);
        meshText.append(rightLabel);

        QGraphicsSimpleTextItem *leftLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/((getMeasurementUnit()==MeasurementUnit::metric)?1000.0:NM*1000.0))
        );
        leftLabel->setFont(labelFont);
        leftLabel->setBrush(MESH_TEXT);
        leftLabel->setPos(-qreal(radius)*zoomScaleFactor+bRect.height()/gap, -bRect.height());
        leftLabel->setData(0, QVariant::fromValue(leftLabel->pos().toPoint()));
        leftLabel->setData(1, QVariant::fromValue(bRect.width()));
        leftLabel->setParentItem(meshTextParent);
        meshText.append(leftLabel);
        dist += scale_step;
    }

    scanIndicator.dots->setPath(scanIndicatorPoints);

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
                    zoomScaleFactor * ((getMeasurementUnit() == MeasurementUnit::metric) ? ZOOM_STEP_KM : ZOOM_STEP_NM) /
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
        azimuthalTextBg->setData(0,QVariant::fromValue(azimuthalTextBg->transform()));
        meshText.append(azimuthalTextBg);
    }
}

void Magnifier::setPpiBackground(const QBrush &brush) {
    setBackgroundBrush(brush);
    ppiScene->setBackgroundBrush(brush);
}

void Magnifier::changePpiScale(quint8 newZoomScale) {
    zoomScale = newZoomScale;
    if (getMeasurementUnit() == MeasurementUnit::metric) {
        zoomScaleFactor = qreal(MAX_LENGTH) / qreal(zoomScaleMaxLengthKm[zoomScale]);
    } else {
        zoomScaleFactor = qreal(MAX_LENGTH) / qreal(zoomScaleMaxLengthNm[zoomScale]);
    }
    repaintMesh();
}

void Magnifier::changeMeasurementUnit(MeasurementUnit newMeasurementUnit) {
    if (newMeasurementUnit == measurementUnit) {
        return;
    }
    measurementUnit = newMeasurementUnit;
}

void Magnifier::drawMesh(bool draw) {
    meshParent->setVisible(draw);
}

void Magnifier::drawDenseMesh(bool draw) {
    meshDenseParent->setVisible(draw);
}

void Magnifier::drawMeshText(bool draw) {
    meshTextParent->setVisible(draw);
}

void Magnifier::wheelEvent(QWheelEvent * event)
{
    int inout = event->delta();
    QMatrix oldMatrix = transform().toAffine();
    QMatrix currMatrix;
    if (inout>0) {
        if (oldMatrix.m11()>1.0 || oldMatrix.m22()>1.0) {
            return;
        }
        setTransform(transform().scale(1.05, 1.05));
        currMatrix = transform().toAffine();
    } else {
        setTransform(transform().scale(0.95, 0.95));
        currMatrix = transform().toAffine();
        if (currMatrix.m11()<origZoom.m11() || currMatrix.m22()<origZoom.m22()) {
            setVisible(false);
        }
    }
    zoom_step(currMatrix);
}

void Magnifier::zoom_step(const QMatrix& newMatrix)
{
    for(QGraphicsItem* textItem: meshText){
        QGraphicsSimpleTextItem *  label = dynamic_cast<QGraphicsSimpleTextItem*>(textItem);
        if (label) {
            QPointF origPos = label->data(0).toPoint();
            qreal origWidth = label->data(1).toReal();
            label->setTransform(
                    QTransform().scale(origZoom.m11() / newMatrix.m11(), origZoom.m11() / newMatrix.m11())
            );
            QPointF curPos = label->pos();
            if (origPos.x() > 0.0) {
                curPos.setX(origPos.x() + (origWidth * (1.0 - origZoom.m22() / newMatrix.m22())));
            }
            curPos.setY(origPos.y() * (origZoom.m22() / newMatrix.m22()));
            label->setPos(curPos);
            continue;
        }
        QGraphicsPathItem *  pathItem = dynamic_cast<QGraphicsPathItem*>(textItem);
        /*
        if (pathItem) {
            QTransform origTrans(pathItem->data(0).value<QTransform>());
            pathItem->setTransform(
                    origTrans.scale(origZoom.m11()/newMatrix.m11(), origZoom.m22()/newMatrix.m22())
            );
            continue;
        }
        */
    }
}

void Magnifier::mousePressEvent(QMouseEvent * event)
{
    savePos = event->globalPos();
    unsetCursor();
    setCursor(Qt::ClosedHandCursor);
    QGraphicsView::mousePressEvent(event);
}

void Magnifier::mouseMoveEvent(QMouseEvent * event)
{
    QGraphicsView *  parentView = qobject_cast<QGraphicsView*>(parent());
    const QPoint delta = event->globalPos()-savePos;
    move(x()+delta.x(), y()+delta.y());
    savePos = event->globalPos();
    QPoint center = frameGeometry().center();
    QPointF moveTo = parentView->mapToScene(center);
    centerOn(moveTo);
}

void Magnifier::mouseReleaseEvent(QMouseEvent * event)
{
    unsetCursor();
    setCursor(Qt::ArrowCursor);
    QGraphicsView::mouseReleaseEvent(event);
}

void Magnifier::saveMatrix(const QMatrix& matrix)
{
    origZoom = matrix;
}

void Magnifier::changeScanIndicator(ScanIndicatorType indicatorType)
{
    if (indicatorType==scanIndicatorType) {
        return;
    }
    if (scanIndicatorType==ScanIndicatorType::line) {
        scanIndicator.line->setVisible(false);
    } else if (scanIndicatorType==ScanIndicatorType::dots) {
        scanIndicator.dots->setVisible(false);
    } else if (scanIndicatorType==ScanIndicatorType::pointer) {
        scanIndicator.pointer->setVisible(false);
    }
    if (indicatorType==ScanIndicatorType::line) {
        scanIndicator.line->setVisible(true);
    } else if (indicatorType==ScanIndicatorType::dots) {
        scanIndicator.dots->setVisible(true);
    } else if (indicatorType==ScanIndicatorType::pointer) {
        scanIndicator.pointer->setVisible(true);
    }
    scanIndicatorType = indicatorType;
}

void Magnifier::scanIndicatorRotate(qreal angle)
{
    scanIndicatorParent->setTransform(
            QTransform().rotate(angle)
    );
}

const QVector<qreal> &Magnifier::getZoomScales() const {
    return (measurementUnit==MeasurementUnit::metric)?zoomScaleMaxLengthKm:zoomScaleMaxLengthNm;
}
