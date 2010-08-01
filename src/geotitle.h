#ifndef GEOTITLE_H
#define GEOTITLE_H

#include <QPointF>
#include <QDataStream>

#include "title.h"

class GeoTitle
{
public:
    GeoTitle() {}
    GeoTitle(Title title, QPointF coordinate) : title(title), coordinate(coordinate) {}
    const Title &getTitle() const { return title; }
    const QPointF &getCoordinate() const { return coordinate; }
private:
    Title title;
    QPointF coordinate;
};

#endif // GEOTITLE_H
