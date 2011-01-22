/*
 * evopedia: An offline Wikipedia reader.
 *
 * Copyright (C) 2010-2011 evopedia developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
