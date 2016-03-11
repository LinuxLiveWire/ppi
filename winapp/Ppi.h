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

#define SCENE_BORDER    20
#define PPI_RADIUS      2000U
#define PPI_SIDE        (2*PPI_RADIUS)

#define SCENE_SIDE      (PPI_SIDE+2*SCENE_BORDER)
#define SCENE_WIDTH     SCENE_SIDE
#define SCENE_HEIGHT    SCENE_WIDTH

#define MINIMAL_ZOOM    100000U
#define ZOOM_STEP       50000U

#define PEN_WIDTH       2.0

#if (RADAR_TYPE==RSP10)
#   define PULSE_LENGTH    75U  // MAX_LENGTH==150km
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
#if (RADAR_TYPE==RSP10)
    xZ100, xZ150, xZMax = xZ150,
#elif ((RADAR_TYPE==P19)||(RADAR_TYPE==TRLK10)||(RADAR_TYPE==EKRAN85))
    xZ100, xZ150, xZ200, xZ250, xZ300,
    xZMax = xZ300,
#elif (RADAR_TYPE==P18)
    xZ100, xZ150, xZ200, xZ250, xZ300, xZ350,
    xZ400, xZ450, xZ500, xZ550, xZ600,
    xZMax = xZ600,
#elif (RADAR_TYPE==P14)
    xZ100, xZ150, xZ200, xZ250, xZ300, xZ350,
    xZ400, xZ450, xZ500, xZ550, xZ600, xZ1000,
    xZMax = xZ1000,
#endif // RADAR_TYPE
    xZMin = xZ100
} PpiZoomScale;
    PpiZoomScale getZoomScale() const { return zoomScale; };
public slots:
    void changePpiScale(PpiZoomScale);
private:
    void scene_initialize();
private:
    QVector<quint32>    zoomScaleMaxLength;
    QGraphicsScene *    ppiScene;
    qreal   desktopScale;
    const quint32   MAX_LENGTH;
    PpiZoomScale    zoomScale;
    qreal   zoomScaleFactor;
};

#endif //PPI_PPI_H
