/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** Changes 2010 by the evopedia developers.
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAP_H
#define MAP_H

#include "evopedia.h"

#include <QObject>
#include <QList>
#include <QRect>
#include <QPoint>
#include <QPointF>
#include <QPainter>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "flickable.h"
#include "tilefetcher.h"

/* TODO1 try if loading from disk cache directly in GUI thread is more efficient */

class SlippyMap: public QObject
{
    Q_OBJECT

    int width;
    int height;
    int zoom;
    bool visible;

    TileFetcher *tileFetcher;

    QPointF m_centerPos; /* in tiles */
    QPoint m_topLeftOffset; /* pixals */
    QRect m_tilesRect;
    QPixmap m_emptyTile;
    QHash<QPoint, QPixmap> m_tilePixmaps;

    void fetchTiles();
    void boundPosition();
protected:
    QRect tileRect(const QPoint &tp);

public:
    SlippyMap(QObject *parent = 0);
    void invalidate();
    void render(QPainter *p, const QRect &rect);
    void pan(const QPoint &delta);
    QPoint titleCoordinateToPixels(const QPointF &c) const;
    QPoint scrollOffset() const;
    void setScrollOffset(const QPoint &offset);
    const QRect &getTilesRect() const { return m_tilesRect; }

    void mouseClicked(const QPoint &pos);

    int getZoom() { return zoom; }
    void setZoom(int zoom);
    void getPosition(qreal &lat, qreal &lng, int &zoom);
    void setPosition(qreal lat, qreal lng, int zoom=-1);
    void resize(int width, int height) {
        this->width = width;
        this->height = height;
        invalidate();
    }

    void hide() { this->visible = false; }
    void show() { this->visible = true; invalidate(); }

    static inline QPointF unproject(const QPointF &xy, int zoom=0);
    static inline QRectF unprojectTileRect(const QPoint &tile, int zoom=0);

    static inline QPointF project(QPointF lnglat, int zoom=0);

private slots:
    void tileLoaded(int zoom, QPoint offset, QImage image);
signals:
    void updated(const QRect &rect);
    void invalidate(const QRect &tilesRect);
    void tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox);
    void mouseClicked(const QPoint &tile, const QPoint &pixelPos);
    void tileNeeded(int zoom, QPoint offset);
};

class ArticleOverlay: public QObject
{
    Q_OBJECT

public:
    ArticleOverlay(SlippyMap *parent);
    bool isComplete();
    bool isEnabled() { return enabled; }

    struct ZoomTile {
        QPoint tile;
        int zoom;
        explicit ZoomTile(QPoint tile=QPoint(), int zoom=0) : tile(tile), zoom(zoom) {}
        bool operator==(const ZoomTile &other) const
        {
            return (tile == other.tile && zoom == other.zoom);
        }
    };
    struct GeoTitleList {
        QList<GeoTitle> list;
        bool complete;
        explicit GeoTitleList(const QList<GeoTitle> &list=QList<GeoTitle>(), bool complete=false)
            : list(list), complete(complete) {}
    };

public slots:
    void setEnabled(bool value) { enabled = value; }

private slots:
    void invalidate(const QRect &tilesRect);
    void tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox);
    void mouseClicked(const QPoint &tile, const QPoint &pixelPos);
    void backendsChanged(QList<LocalArchive *>);

private:
    GeoTitleList getTitles(const QRectF &rect, int maxTitles);
    void showNearTitleList(const QList<Title> &t);

    bool enabled;
    QHash<ZoomTile, GeoTitleList> titles;
    QPixmap wikipediaIcon;
    SlippyMap *slippyMap;
};

#endif // MAP_H
