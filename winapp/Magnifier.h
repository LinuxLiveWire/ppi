//
// Created by sergey on 21.03.16.
//

#ifndef PPI_MAGNIFIER_H
#define PPI_MAGNIFIER_H

#define ZOOM_VIEW_KEY   255

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMatrix>

class MagnifierWindow: public QGraphicsView {
Q_OBJECT
public:
    MagnifierWindow(QWidget* parent=0);
protected:
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
public:
    void saveMatrix(const QMatrix&);
private:
    QPoint savePos;
    QMatrix minZoom;  // zoom level below which window is closed
};

class Magnifier: public QGraphicsView {
    Q_OBJECT
signals:
    void updateZoomedRegion(const QRectF&);
public:
    Magnifier(QWidget* parent=0);
    QRectF getCurrrentlyVisibleRegion() const;
    void update(bool yesno) { doUpdate=yesno; }
protected:
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void scrollContentsBy(int , int) override;
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;
private:
    bool doUpdate;
};

#endif //PPI_MAGNIFIER_H
