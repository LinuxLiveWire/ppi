//
// Created by sergey on 10.03.16.
//
#include <QDebug>
#include <QRectF>
#include <QGraphicsPathItem>
#include <QApplication>
#include <QDesktopWidget>
#include <QOpenGLWidget>
#include <QGLWidget>

#include "Ppi.h"

QVariant ZoomView::itemChange(GraphicsItemChange  change, const QVariant & value)
{
    return QGraphicsItem::itemChange(change, value);
}

void ZoomView::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    emit viewChanged(sceneBoundingRect()); //&Qt::LeftButton
    QGraphicsRectItem::mouseMoveEvent(event);
}

Ppi::Ppi(QWidget *parent) :
        QGraphicsView(parent), scanIndicatorType(line),
        MAX_LENGTH(PULSE_LENGTH * PPI_RADIUS), desktopScale(1.0), measurementUnit(metric),
        scanIndicator {nullptr, nullptr, nullptr}
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
//    QGLFormat fmt;
//    fmt.setSampleBuffers( true );
//    fmt.setSamples( 2 );
//    fmt.setDoubleBuffer( true );
//    fmt.setAlpha ( true );
//    fmt.setDepth( true );
//    fmt.setDirectRendering( true );

//    setViewport(new QGLWidget(QGLFormat(
//            QGL::SampleBuffers|QGL::DirectRendering|QGL::HasOverlay|QGL::StencilBuffer|QGL::AccumBuffer|QGL::Rgba
//    )));
    setRenderHint( QPainter::Antialiasing, true );
    setRenderHint( QPainter::TextAntialiasing, true );
//    setRenderHint( QPainter::HighQualityAntialiasing, true );
    setRenderHint( QPainter::SmoothPixmapTransform, false );
//    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    scene_initialize();
    setPpiBackground(Qt::black);
    repaintMesh();
    magnifier = new MagnifierWindow(this);
    magnifier->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    magnifier->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    magnifier->setRenderHint(QPainter::Antialiasing, true);
    magnifier->setScene(scene());
    magnifier->setVisible(false);
    magnifier->setWindowFlags(magnifier->windowFlags()|Qt::Window|Qt::FramelessWindowHint);

    zoomWindow = new Magnifier(this);
    zoomWindow->setFixedSize(350, 350);
    zoomWindow->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    zoomWindow->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    zoomWindow->setRenderHint(QPainter::Antialiasing, true);
    zoomWindow->setScene(scene());
    zoomWindow->setVisible(false);
    createConnection();
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

    zoomFrame = new ZoomView();
    zoomFrame->setPen(QPen(QColor(255, 64, 64), PEN_WIDTH_THICK, Qt::SolidLine));
    zoomFrame->setData(0, QVariant::fromValue(ZOOM_VIEW_KEY));
    ppiScene->addItem(zoomFrame);
    zoomFrame->setFlags(zoomFrame->flags()|QGraphicsItem::ItemIsMovable);
    zoomFrame->setVisible(false);

    QRect geom = qApp->desktop()->availableGeometry(this);
    desktopScale = qreal(geom.height() + geom.y()) / sceneRect.height();
    setRenderHints(renderHints() | QPainter::Antialiasing);
    setScene(ppiScene);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scale(desktopScale, desktopScale);
}

void Ppi::createConnection()
{
    connect(zoomWindow, &Magnifier::updateZoomedRegion, [=](const QRectF& view){
        zoomFrame->setRect(QRectF(view.x()-zoomFrame->pos().x(), view.y()-zoomFrame->pos().y(), view.width(), view.height()));
    });
    connect(zoomFrame, &ZoomView::viewChanged, [=](const QRectF& rect){
        zoomWindow->update(false);
        zoomWindow->fitInView(rect); //-QMargins(8, 8, 8, 8)
        zoomWindow->update(true);
    });
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
    setBackgroundBrush(brush);
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

void Ppi::changeScanIndicator(ScanIndicatorType indicatorType)
{
    if (indicatorType==scanIndicatorType) {
        return;
    }
    if (scanIndicatorType==line) {
        scanIndicator.line->setVisible(false);
    } else if (scanIndicatorType==dots) {
        scanIndicator.dots->setVisible(false);
    } else if (scanIndicatorType==pointer) {
        scanIndicator.pointer->setVisible(false);
    }
    if (indicatorType==line) {
        scanIndicator.line->setVisible(true);
    } else if (indicatorType==dots) {
        scanIndicator.dots->setVisible(true);
    } else if (indicatorType==pointer) {
        scanIndicator.pointer->setVisible(true);
    }
    scanIndicatorType = indicatorType;
}

void Ppi::scanIndicatorRotate(qreal angle)
{
    scanIndicatorParent->setTransform(
            QTransform().rotate(angle)
    );
}

void Ppi::showZoom(bool show)
{
    if (show) {
        zoomWindow->setVisible(true);
        zoomWindow->move(0, height()-350);
        zoomWindow->fitInView(
                sceneRect().center().x()-200, sceneRect().center().y()-200,
                400, 400,
                Qt::KeepAspectRatio
        );
        zoomFrame->setRect(
                QRectF(sceneRect().center().x()-200, sceneRect().center().y()-200,
                400, 400)// - QMargins(-3, -3, -3, -3)
        );
        zoomFrame->setVisible(true);
    } else {
        zoomWindow->setVisible(false);
        zoomFrame->setVisible(false);
    }
}

void Ppi::resizeEvent(QResizeEvent * event)
{
    zoomWindow->move(0, height()-350);
    QGraphicsView::resizeEvent(event);
}

void Ppi::wheelEvent(QWheelEvent * event)
{
    if (event->delta()>0) {
        if (!magnifier->isVisible()) {
            magnifier->setVisible(true);
        }
        magnifier->move(event->pos().x()-magnifier->width()/2, event->pos().y()-magnifier->height()/2);
        QRect magnWin = magnifier->frameGeometry();
        QRectF map = mapToScene(magnWin).boundingRect();
        magnifier->fitInView(map);
    } else {
        if (magnifier->isVisible()) {
            magnifier->setVisible(false);
        }
    }
}
