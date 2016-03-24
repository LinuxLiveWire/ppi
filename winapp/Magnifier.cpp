//
// Created by sergey on 21.03.16.
//

#include <QTransform>
#include <QGraphicsItem>
#include <QDebug>
#include "Magnifier.h"

MagnifierWindow::MagnifierWindow(QWidget * parent):
    QGraphicsView(parent)
{
    setMinimumSize(QSize(100, 100));
    setMaximumSize(QSize(300, 300));
    setTransformationAnchor( QGraphicsView::AnchorViewCenter );
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setStyleSheet("QFrame { border: 1px solid red; }");
    //setFrameStyle(QFrame::NoFrame);
}

void MagnifierWindow::wheelEvent(QWheelEvent * event)
{
    int inout = event->delta();
    if (inout>0) {
        QMatrix currMatrix = transform().toAffine();
        if (currMatrix.m11()>1.0 || currMatrix.m22()>1.0) {
            return;
        }
        setTransform(transform().scale(1.05, 1.05));
    } else {
        setTransform(transform().scale(0.95, 0.95));
        QMatrix current = transform().toAffine();
        if (current.m11()<minZoom.m11() || current.m22()<minZoom.m22()) {
            setVisible(false);
        }
    }
}

void MagnifierWindow::mousePressEvent(QMouseEvent * event)
{
    savePos = event->globalPos();
    unsetCursor();
    setCursor(Qt::ClosedHandCursor);
    QGraphicsView::mousePressEvent(event);
}

void MagnifierWindow::mouseMoveEvent(QMouseEvent * event)
{
    QGraphicsView *  parentView = qobject_cast<QGraphicsView*>(parent());
    const QPoint delta = event->globalPos()-savePos;
    move(x()+delta.x(), y()+delta.y());
    savePos = event->globalPos();
    QPoint center = frameGeometry().center();
    QPointF moveTo = parentView->mapToScene(center);
    centerOn(moveTo);
}

void MagnifierWindow::mouseReleaseEvent(QMouseEvent * event)
{
    unsetCursor();
    setCursor(Qt::ArrowCursor);
    QGraphicsView::mouseReleaseEvent(event);
}

void MagnifierWindow::saveMatrix(const QMatrix& matrix)
{
    minZoom = matrix;
}

Magnifier::Magnifier(QWidget* parent):
    QGraphicsView(parent), doUpdate(true)
{
    setTransformationAnchor( QGraphicsView::AnchorUnderMouse );
    setDragMode( QGraphicsView::ScrollHandDrag );
}

void Magnifier::wheelEvent(QWheelEvent * event)
{
    int inout = event->delta();
    if (inout>0) {
        setTransform(transform().scale(1.01, 1.01));
    } else {
        setTransform(transform().scale(0.99, 0.99));
    }
}

QRectF Magnifier::getCurrrentlyVisibleRegion() const
{
    QPointF topLeft = mapToScene (0, 0);
    QPointF bottomRight = mapToScene ( this->width(), this->height() );
    return QRectF ( topLeft, bottomRight );
}

void Magnifier::scrollContentsBy(int dx, int dy)
{
    QRectF visibleRegion = getCurrrentlyVisibleRegion();
    if (doUpdate) {
        emit updateZoomedRegion(visibleRegion);
    }
    QGraphicsView::scrollContentsBy(dx, dy);
}

void Magnifier::mousePressEvent(QMouseEvent * event)
{
    QList<QGraphicsItem*> underView = items(event->pos());
    foreach(QGraphicsItem* item, underView) {
            int data = item->data(0).toInt();
            if (data==ZOOM_VIEW_KEY) {
                item->setFlags(item->flags()&~QGraphicsItem::ItemIsMovable);
                break;
            }
    }
    QGraphicsView::mousePressEvent(event);
}

void Magnifier::mouseReleaseEvent(QMouseEvent * event)
{
    QList<QGraphicsItem*> underView = items(event->pos());
    foreach(QGraphicsItem* item, underView) {
        int data = item->data(0).toInt();
        if (data==ZOOM_VIEW_KEY) {
            item->setFlags(item->flags()|QGraphicsItem::ItemIsMovable);
            break;
        }
    }
    QGraphicsView::mouseReleaseEvent(event);
}
