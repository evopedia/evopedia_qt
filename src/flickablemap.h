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

#ifndef FLICKABLEMAP_H
#define FLICKABLEMAP_H

#include <QWidget>

#include "flickable.h"
#include "map.h"

class FlickableMap : public QWidget, public Flickable
{
    Q_OBJECT
public:
    explicit FlickableMap(QWidget *parent = 0);
    void setPosition(qreal lat, qreal lng, int zoom=-1);
    void getPosition(qreal &lat, qreal &lng, int &zoom);

public slots:
    void zoomIn();
    void zoomOut();
    void overlaysEnable(bool value);

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

    virtual QPoint scrollOffset() const {
        return map->scrollOffset();
    }

    virtual void setScrollOffset(const QPoint &offset) {
        map->setScrollOffset(offset);
    }

private slots:
    void updateMap(const QRect &r);

private:
    SlippyMap *map;
    ArticleOverlay *articleOverlay;
};

#endif // FLICKABLEMAP_H
