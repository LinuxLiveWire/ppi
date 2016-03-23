//
// Created by sergey on 21.03.16.
//

#include <QTransform>
#include <QGraphicsItem>
#include <QDebug>
#include "Magnifier.h"

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
