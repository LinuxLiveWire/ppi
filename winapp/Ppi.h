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

#define P18     1
#define P19     2
#define TRLK10  3
#define P14     4
#define EKRAN85 5
#define RSP10   6

#define SCENE_BORDER    30
#define PPI_RADIUS      2000U
#define PPI_SIDE        (2*PPI_RADIUS)

#define SCENE_SIDE      (PPI_SIDE+2*SCENE_BORDER)
#define SCENE_WIDTH     SCENE_SIDE
#define SCENE_HEIGHT    SCENE_WIDTH

#define NM  1.852              // Meters in nautical mile
#define KM2NM(km) ((km)/NM)    // Killometers to nautical miles
#define NM2KM(nm) ((nm)*NM)    // Nautical miles to killometers

#define MINIMAL_ZOOM_KM    100000.0
#define MINIMAL_ZOOM_NM    NM2KM(50000.0)
#define ZOOM_STEP_KM       50000.0
#define ZOOM_STEP_NM       NM2KM(50000.0)
#define ZOOM_DENSE_STEP_KM   10000.0
#define ZOOM_DENSE_STEP_NM   NM2KM(10000.0)

#define PEN_WIDTH_THIN      0.7
#define PEN_WIDTH           1.0
#define PEN_WIDTH_THICK     1.4

#define	MESH_COLOR      	QColor(238, 238, 238, 255)
#define	MESH_TEXT			QColor(136, 136, 136, 255)

#if (RADAR_TYPE==RSP10)
#   define PULSE_LENGTH    75U  // MAX_LENGTH==PPI_RADIUS*PULSE_LENGTH==150km
#elif ((RADAR_TYPE==P19)||(RADAR_TYPE==TRLK10)||(RADAR_TYPE==EKRAN85))
#   define PULSE_LENGTH    150U // MAX_LENGTH==300km
#elif (RADAR_TYPE==P18)
#   define PULSE_LENGTH    300U // MAX_LENGTH==600km
#elif (RADAR_TYPE==P14)
#   define PULSE_LENGTH    500U // MAX_LENGTH==1000km
#endif // RADAR_TYPE


class Ppi: public QGraphicsView {
Q_OBJECT
public:
    Ppi(QWidget* parent=0);
typedef enum {
    metric, imperial
} MeasurementUnit;
typedef enum {
    line, pointer, dots
} ScanIndicatorType;
    MeasurementUnit getMeasurementUnit() const { return measurementUnit; }
    const QVector<qreal>& getZoomScales() const;
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
    MagnifierWindow *  magnifier;
};

#endif //PPI_PPI_H
