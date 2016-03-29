//
// Created by sergey on 21.03.16.
//

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsItem>
#include "Magnifier.h"

#define SCAN_POINT_SIZE     6

// Data key enums - for internal use only
typedef enum {
    PEN_WIDTH_KEY, AZIMUTH_KEY,
    GAP_KEY, RADIUS_KEY, RECT_KEY,
    SCAN_POINTS_KEY
}  DataKey;

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
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
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
    meshDenseParent->setZValue(10.0);
    meshDenseParent->setParentItem(meshParent);
    meshTextParent = new QGraphicsRectItem();
    meshTextParent->setFlags(
            meshTextParent->flags() | \
            QGraphicsItem::ItemDoesntPropagateOpacityToChildren | \
            QGraphicsItem::ItemHasNoContents
    );
    meshTextParent->setOpacity(1.0);
    meshTextParent->setZValue(20.0);
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
    scanIndicator.pointer->setData(PEN_WIDTH_KEY, QVariant::fromValue(PEN_WIDTH_THICK));
    scanIndicator.line = new QGraphicsLineItem(
            QLineF(
                    sceneRect.center().x(), sceneRect.center().y(),
                    sceneRect.center().x(), -qreal(PPI_RADIUS)
            ),
            scanIndicatorParent
    );
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
    QList<QVariant> scanPointsPos;
    QMatrix currZoom = transform().toAffine();
    qreal factor = origZoom.m11()/currZoom.m11();
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
        QGraphicsEllipseItem *zoneRing = ppiScene->addEllipse(zoneRect, QPen(MESH_COLOR, penWidth*factor, Qt::SolidLine));
        zoneRing->setOpacity(1.0);
        zoneRing->setParentItem(meshParent);
        zoneRing->setZValue(15.0);
        zoneRing->setData(PEN_WIDTH_KEY, QVariant::fromValue(penWidth));
        meshFiber.append(zoneRing);

        scanIndicatorPoints.addEllipse(
                QPointF(sceneRect().center().x(), -qreal(radius)*zoomScaleFactor),
                SCAN_POINT_SIZE*factor, SCAN_POINT_SIZE*factor
        );
        scanPointsPos.append(QVariant::fromValue(-qreal(radius)*zoomScaleFactor));

        for (quint32 j = 0; min_scale_dist<dist; ++j) {
            quint32 min_radius = min_scale_dist / PULSE_LENGTH;
            penWidth = PEN_WIDTH_THIN;
            QRectF minZoneRect(
                    -qreal(min_radius)*zoomScaleFactor, -qreal(min_radius)*zoomScaleFactor,
                    2 * qreal(min_radius)*zoomScaleFactor, 2 * qreal(min_radius)*zoomScaleFactor
            );
            QGraphicsEllipseItem *minZoneRing = ppiScene->addEllipse(
                    minZoneRect, QPen(MESH_COLOR, penWidth*factor, Qt::DotLine)
            );
            minZoneRing->setOpacity(1.0);
            minZoneRing->setParentItem(meshDenseParent);
            minZoneRing->setZValue(15.0);
            minZoneRing->setData(PEN_WIDTH_KEY, QVariant::fromValue(penWidth));
            meshFiber.append(minZoneRing);
            min_scale_dist += min_scale_step;
        }
        min_scale_dist = dist + min_scale_step;

        qreal gap = (dist==stopZoomScale)?2.0:1.0; // Last text element less shifted
        QGraphicsSimpleTextItem *rightLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/((getMeasurementUnit()==MeasurementUnit::metric)?1000.0:NM*1000.0))
        );
        QFont labelFont = rightLabel->font();
        labelFont.setPointSize(labelFont.pointSize()/desktopScale);
        rightLabel->setFont(labelFont);
        rightLabel->setBrush(MESH_TEXT);
        QRectF bRect = rightLabel->boundingRect();
        rightLabel->setTransform(
                QTransform().translate(sceneRect().center().x(), sceneRect().center().y()).\
                        scale(factor, factor).
                        translate(qreal(radius)*zoomScaleFactor/factor-(bRect.width()+gap*bRect.height()/4.), 0)
        );
        rightLabel->setData(RECT_KEY, QVariant::fromValue(bRect));
        rightLabel->setData(GAP_KEY, QVariant::fromValue(gap));
        rightLabel->setData(RADIUS_KEY, QVariant::fromValue(radius));
        rightLabel->setParentItem(meshTextParent);
        meshText.append(rightLabel);

        QGraphicsSimpleTextItem *leftLabel = ppiScene->addSimpleText(
                QString("%1").arg(dist/((getMeasurementUnit()==MeasurementUnit::metric)?1000.0:NM*1000.0))
        );
        leftLabel->setFont(labelFont);
        leftLabel->setBrush(MESH_TEXT);
        leftLabel->setTransform(
                QTransform().translate(sceneRect().center().x(), sceneRect().center().y()).\
                        scale(factor, factor).
                        translate(-qreal(radius)*zoomScaleFactor/factor+gap*bRect.height()/4.0, -bRect.height())
        );
        leftLabel->setData(RECT_KEY, QVariant::fromValue(bRect));
        leftLabel->setData(GAP_KEY, QVariant::fromValue(gap));
        leftLabel->setData(RADIUS_KEY, QVariant::fromValue(radius));
        leftLabel->setParentItem(meshTextParent);
        meshText.append(leftLabel);
        dist += scale_step;
    }

    scanIndicator.dots->setPath(scanIndicatorPoints);
    scanIndicator.dots->setData(SCAN_POINTS_KEY, QVariant(scanPointsPos));

    qreal scanPenWidth = PEN_WIDTH_THICK;
    scanIndicator.line->setPen(QPen(MESH_COLOR, scanPenWidth*factor, Qt::SolidLine));
    scanIndicator.line->setData(PEN_WIDTH_KEY, QVariant::fromValue(scanPenWidth));

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
                    zoomScaleFactor*((getMeasurementUnit()==MeasurementUnit::metric)?ZOOM_STEP_KM:ZOOM_STEP_NM)/
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
        azimuthalLine->setPen(QPen(MESH_COLOR, penWidth*factor, Qt::SolidLine));
        azimuthalLine->setTransform(
                azimuthalLine->transform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                rotate(azimuth).\
                translate(-sceneRect().center().x(), -sceneRect().center().y())
        );
        azimuthalLine->setOpacity(1.0);
        azimuthalLine->setParentItem(meshParent);
        azimuthalLine->setZValue(15.0); // !!! - set value less than for all meshTextParent childs
        azimuthalLine->setData(PEN_WIDTH_KEY, QVariant::fromValue(penWidth));
        meshFiber.append(azimuthalLine);
        azimuthalTextBg = new QGraphicsPathItem();
        penWidth = PEN_WIDTH_THIN;
        azimuthalTextBg->setOpacity(1.0);
        //azimuthalTextBg->setBrush(scene()->backgroundBrush());
        azimuthalTextBg->setBrush(Qt::black);
        azimuthalTextBg->setPen(QPen(MESH_COLOR, penWidth*factor, Qt::SolidLine));
        azimuthalTextBg->setParentItem(meshTextParent);
        azimuthalTextBg->setData(AZIMUTH_KEY, QVariant::fromValue(azimuth));
        azimuthalTextBg->setZValue(30.0);
        azimuthalTextBg->setData(PEN_WIDTH_KEY, QVariant::fromValue(penWidth));

        QPainterPath roundRect;
        roundRect.addRoundedRect(br - QMargins(-4, 2, -4, 2), 24, 24);
        azimuthalTextBg->setPath(roundRect);

        azimuthalText = new QGraphicsSimpleTextItem(QString("%1").arg(azimuth));
        azimuthalText->setParentItem(azimuthalTextBg);
        azimuthalText->setFont(labelFont);
        azimuthalText->setBrush(MESH_TEXT);
        azimuthalText->setZValue(35.0);
        azimuthalText->setPos((br.width()-azimuthalText->boundingRect().width())/2, 0);

        if (azimuth >= 270 || azimuth <= 90) {
            azimuthalTextBg->setTransform(
                    azimuthalText->transform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                scale(factor, factor).\
                rotate(azimuth).\
                translate(sceneRect().center().x() - br.width()/2.0,
                          (sceneRect().center().y() - PPI_RADIUS/factor - br.height()/2))
            );
        } else {
            azimuthalTextBg->setTransform(
                    azimuthalText->transform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                scale(factor, factor).\
                rotate(180.+azimuth).\
                translate(sceneRect().center().x() - br.width() / 2.0,
                          sceneRect().center().y() + PPI_RADIUS/factor - br.height()/2)
            );
        }
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
    QGraphicsSimpleTextItem textSample(QString("%1").arg(360)); // for font metric only
    QFont labelFont = textSample.font();
    labelFont.setPointSize(labelFont.pointSize()/desktopScale);
    textSample.setFont(labelFont);
    QRectF br = textSample.boundingRect(); // Rescaled text border
    qreal factor = origZoom.m11()/newMatrix.m11();
    qreal penWidth;
    for(QGraphicsItem* geomItem: meshFiber){
        QPen currPen;
        QGraphicsEllipseItem *  zoneRing = dynamic_cast<QGraphicsEllipseItem*>(geomItem);
        if(zoneRing){
            penWidth = zoneRing->data(PEN_WIDTH_KEY).toReal();
            currPen = zoneRing->pen();
            currPen.setWidthF(penWidth*factor);
            zoneRing->setPen(currPen);
            continue;
        }
        QGraphicsLineItem *  azimuthBeam = dynamic_cast<QGraphicsLineItem*>(geomItem);
        if(azimuthBeam){
            penWidth = azimuthBeam->data(PEN_WIDTH_KEY).toReal();
            currPen = azimuthBeam->pen();
            currPen.setWidthF(penWidth*factor);
            azimuthBeam->setPen(currPen);
            continue;
        }
    }

    for(QGraphicsItem* textItem: meshText){
        QGraphicsSimpleTextItem *  label = dynamic_cast<QGraphicsSimpleTextItem*>(textItem);
        if (label) {
            QTransform currMatrix(label->transform());
            quint32 radius = label->data(RADIUS_KEY).toUInt();
            qreal gap = label->data(GAP_KEY).toReal();
            QRectF origRect = label->data(RECT_KEY).toRectF();
            if (currMatrix.m31()>0) {
                label->setTransform(
                        QTransform().translate(sceneRect().center().x(), sceneRect().center().y()).\
                        scale(origZoom.m11()/newMatrix.m11(), origZoom.m22()/newMatrix.m22()).\
                        translate(qreal(radius)*zoomScaleFactor/factor-(origRect.width()+gap*origRect.height()/4.), 0)
                );
            } else {
                label->setTransform(
                        QTransform().translate(sceneRect().center().x(), sceneRect().center().y()).\
                        scale(origZoom.m11()/newMatrix.m11(), origZoom.m22()/newMatrix.m22()).\
                        translate(-qreal(radius)*zoomScaleFactor/factor+gap*origRect.height()/4.0, -origRect.height())
                );
            }
            continue;
        }
        QGraphicsPathItem *  pathItem = dynamic_cast<QGraphicsPathItem*>(textItem);
        if (pathItem) {
            qreal azimuth = pathItem->data(AZIMUTH_KEY).toReal();
            qreal penWidth = pathItem->data(PEN_WIDTH_KEY).toReal();
            if (azimuth >= 270 || azimuth <= 90) {
                pathItem->setTransform(
                        QTransform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                scale(origZoom.m11() / newMatrix.m11(), origZoom.m11() / newMatrix.m11()).\
                rotate(azimuth).\
                translate(sceneRect().center().x() - br.width() / 2.0,
                          sceneRect().center().y() - (PPI_RADIUS)/(origZoom.m11()/newMatrix.m11()) - br.height()/2)
                );
            } else {
                pathItem->setTransform(
                        QTransform().\
                translate(sceneRect().center().x(), sceneRect().center().y()).\
                scale(origZoom.m11()/newMatrix.m11(), origZoom.m11()/newMatrix.m11()).\
                rotate(180.+azimuth).\
                translate(sceneRect().center().x()-br.width() / 2.0,
                          sceneRect().center().y() + (PPI_RADIUS)/(origZoom.m11()/newMatrix.m11()) - br.height()/2)
                );
            }
            QPen currPen = pathItem->pen();
            currPen.setWidthF(penWidth*factor);
            pathItem->setPen(currPen);
            continue;
        }
    }
    auto scanDots = scanIndicator.dots->data(SCAN_POINTS_KEY).toList();
    QPainterPath scanIndicatorPoints;
    for(QVariant dot: scanDots){
        qreal y = dot.toReal();
        scanIndicatorPoints.addEllipse(
                QPointF(sceneRect().center().x(), y),
                SCAN_POINT_SIZE*factor, SCAN_POINT_SIZE*factor
        );
    }
    scanIndicator.dots->setPath(scanIndicatorPoints);

    QPen scanPen = scanIndicator.line->pen();
    qreal scanPenWidth = scanIndicator.line->data(PEN_WIDTH_KEY).toReal();
    scanPen.setWidthF(scanPenWidth*factor);
    scanIndicator.line->setPen(scanPen);
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
