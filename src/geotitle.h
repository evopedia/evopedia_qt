#ifndef GEOTITLE_H
#define GEOTITLE_H

#include <QPointF>
#include <QDataStream>
#include <QPair>

#include "title.h"

class GeoTitle
{
public:
    GeoTitle() {}
    GeoTitle(Title title, QPointF coordinate) : title(title), coordinate(coordinate) {}
    const Title &getTitle() const { return title; }
    const QPointF &getCoordinate() const { return coordinate; }

    static bool nearerThan(const QPair<GeoTitle, float> &t1, const QPair<GeoTitle, float> &t2)
    {
        return t1.second < t2.second;
    }

private:
    Title title;
    QPointF coordinate;
};

#endif // GEOTITLE_H
