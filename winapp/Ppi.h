//
// Created by sergey on 10.03.16.
//

#ifndef PPI_PPI_H
#define PPI_PPI_H

#include <QGraphicsView>
#include <QGraphicsScene>

class Ppi: public QGraphicsView {
Q_OBJECT
public:
    Ppi(QWidget* parent=0);
private:
    void scene_initialize();
private:
    QGraphicsScene *    ppiScene;
    qreal   desktopScale;
};

#endif //PPI_PPI_H
