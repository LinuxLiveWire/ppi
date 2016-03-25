//
// Created by sergey on 21.03.16.
//

#ifndef PPI_MAGNIFIER_H
#define PPI_MAGNIFIER_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMatrix>

#include "ppidefs.h"

class Magnifier: public QGraphicsView {
Q_OBJECT
public:
    Magnifier(QWidget* parent=0);
protected:
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
public slots:
    void changePpiScale(quint8);
    void changeMeasurementUnit(MeasurementUnit);
    void setPpiBackground(const QBrush &brush);
    void drawMesh(bool);
    void drawDenseMesh(bool);
    void drawMeshText(bool);
public:
    void saveMatrix(const QMatrix&);
    void changeScanIndicator(ScanIndicatorType);
    void scanIndicatorRotate(qreal);
    void repaintMesh();
    quint8 getZoomScale() const { return zoomScale; }
    ScanIndicatorType getScanIndicatorType() const { return scanIndicatorType; }
    MeasurementUnit getMeasurementUnit() const { return measurementUnit; }
    const QVector<qreal>& getZoomScales() const;
private:
    void scene_initialize();
    void zoom_step(const QMatrix&);
private:
    ScanIndicatorType   scanIndicatorType;
    const quint32   MAX_LENGTH; // The maximum possible range of data display
    // for current type of radar (compile time)
    QVector<qreal>  zoomScaleMaxLengthKm; // Metric table of scale
    QVector<qreal>  zoomScaleMaxLengthNm; // Imperial table of scale
    qreal   desktopScale; // Coefficient for accurate fitting in current desktop geometry
    quint8  zoomScale;  // Current scale
    qreal   zoomScaleFactor; // Coefficient for current desktop scale
    MeasurementUnit measurementUnit; // Current measurement unit (Metric or Imperial)
    QGraphicsScene *    ppiScene;
    QGraphicsItem *     meshParent;
    QGraphicsItem *     meshDenseParent;
    QGraphicsItem *     meshTextParent;
    QGraphicsItem *     scanIndicatorParent;
    struct {
        QGraphicsLineItem *  line;
        QGraphicsPathItem *  dots;
        QGraphicsPolygonItem* pointer;
    } scanIndicator;
    QVector<QGraphicsItem*> meshFiber; // Container for mesh elements
    QVector<QGraphicsItem*> meshText; // Container for mesh text
    QPoint savePos;
    QMatrix origZoom;  // zoom level below which window is closed
};

#endif //PPI_MAGNIFIER_H
