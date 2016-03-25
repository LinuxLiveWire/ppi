//
// Created by sergey on 24.03.16.
//

#ifndef PPI_PPIDEFS_H
#define PPI_PPIDEFS_H

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

enum class MeasurementUnit {
    metric, imperial
};

enum class ScanIndicatorType {
    line, pointer, dots
};

#endif //PPI_PPIDEFS_H
