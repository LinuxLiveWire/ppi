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

#include <QGraphicsView>
#include <QGraphicsScene>
#include "QVector"

#define P18     1
#define P19     2
#define TRLK10  3
#define P14     4
#define EKRAN85 5
#define RSP10   6

#define SCENE_BORDER    40
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

#define PEN_WIDTH_THIN      1.0
#define PEN_WIDTH           1.5
#define PEN_WIDTH_THICK     2.0

#define	MESH_COLOR      	QColor( 238, 238, 238 )
#define	MESH_TEXT			QColor( 136, 136, 136 )

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
    MeasurementUnit getMeasurementUnit() const { return measurementUnit; }
    const QVector<qreal>& getZoomScales() const;
public slots:
    void changePpiScale(quint8);
    void changeMeasurementUnit(MeasurementUnit);
    void setPpiBackground(const QBrush&);
    void drawMesh(bool);
    void drawMeshText(bool);
public:
    void repaintMesh();
    quint8 getZoomScale() const { return zoomScale; }
private:
    void scene_initialize();
private:
    QVector<qreal>    zoomScaleMaxLengthKm;
    QVector<qreal>    zoomScaleMaxLengthNm;
    QGraphicsScene *    ppiScene;
    qreal   desktopScale;
    const quint32   MAX_LENGTH;
    quint8    zoomScale;
    MeasurementUnit measurementUnit;
    qreal   zoomScaleFactor;
    QGraphicsItem *  meshParent;
    QGraphicsItem *  meshTextParent;
    QVector<QGraphicsItem*> radialMesh;
    QVector<QGraphicsItem*> radialText;
};

#endif //PPI_PPI_H
