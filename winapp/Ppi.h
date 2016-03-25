//
// Created by sergey on 10.03.16.
//

/* *** Pulse length/sample length ***
 * P14 - 10us/3.3us
 * P18 - 6us/2us
 * P19 - 3us/1us
 * TrLk- 3us/1us
 * Ekran85 - 3us/1us
 * Rsp10 - 1.5us/0.5us
 * */

#ifndef PPI_PPI_H
#define PPI_PPI_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QVector>

#include "Magnifier.h"

class Ppi: public QGraphicsView {
Q_OBJECT
public:
    Ppi(QWidget* parent=0);
public slots:
    void changePpiScale(quint8);
    void changeMeasurementUnit(MeasurementUnit);
    void setPpiBackground(const QBrush&);
    void drawMesh(bool);
    void drawDenseMesh(bool);
    void drawMeshText(bool);
    void changeScanIndicator(ScanIndicatorType);
    void scanIndicatorRotate(qreal);
public:
    void repaintMesh();
    quint8 getZoomScale() const { return zoomScale; }
    ScanIndicatorType getScanIndicatorType() const { return scanIndicatorType; }
    MeasurementUnit getMeasurementUnit() const { return measurementUnit; }
    const QVector<qreal>& getZoomScales() const;
protected:
    void resizeEvent ( QResizeEvent * );
    void createConnection();
    virtual void wheelEvent(QWheelEvent *) override;
private:
    void scene_initialize();
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
    Magnifier *  magnifier;
};

#endif //PPI_PPI_H
