/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
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

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QMessageBox>

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif

#if defined (Q_OS_SYMBIAN)
#include "sym_iap_util.h"
#endif

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "map.h"
#include "utils.h"
#include "storagebackend.h"

uint qHash(const QPoint& p)
{
    return p.x() * 17 ^ p.y();
}

// tile size in pixels
const int tdim = 256;

QPointF tileForCoordinate(qreal lat, qreal lng, int zoom)
{
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal tx = (lng + 180.0) / 360.0;
    qreal ty = (1.0 - log(tan(lat * M_PI / 180.0) +
                          1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0;
    return QPointF(tx * zn, ty * zn);
}

qreal longitudeFromTile(qreal tx, int zoom)
{
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal lat = tx / zn * 360.0 - 180.0;
    return lat;
}

qreal latitudeFromTile(qreal ty, int zoom)
{
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal n = M_PI - 2 * M_PI * ty / zn;
    qreal lng = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
    return lng;
}

QRectF coordinatesFromTile(const QPoint &tile, int zoom)
{
    QPointF bottomLeft(longitudeFromTile(tile.x(), zoom),
                    latitudeFromTile(tile.y(), zoom));
    QPointF topRight(longitudeFromTile(tile.x() + 1, zoom),
                    latitudeFromTile(tile.y() + 1, zoom));

    return QRectF(bottomLeft, topRight);
}


SlippyMap::SlippyMap(QObject *parent)
    : QObject(parent)
    , width(400)
    , height(300)
    , zoom(15)
    , latitude(59.9138204)
    , longitude(10.7387413) {
    m_emptyTile = QPixmap(tdim, tdim);
    m_emptyTile.fill(Qt::lightGray);

    QNetworkDiskCache *cache = new QNetworkDiskCache;
    cache->setCacheDirectory(QDesktopServices::storageLocation
                             (QDesktopServices::CacheLocation));
    m_manager.setCache(cache);
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));
}

void SlippyMap::invalidate() {
    if (width <= 0 || height <= 0)
        return;

    QPointF ct = tileForCoordinate(latitude, longitude, zoom);
    qreal tx = ct.x();
    qreal ty = ct.y();

    // top-left corner of the center tile
    int xp = width / 2 - (tx - floor(tx)) * tdim;
    int yp = height / 2 - (ty - floor(ty)) * tdim;

    // first tile vertical and horizontal
    int xa = (xp + tdim - 1) / tdim;
    int ya = (yp + tdim - 1) / tdim;
    int xs = static_cast<int>(tx) - xa;
    int ys = static_cast<int>(ty) - ya;

    // offset for top-left tile
    m_offset = QPoint(xp - xa * tdim, yp - ya * tdim);

    // last tile vertical and horizontal
    int xe = static_cast<int>(tx) + (width - xp - 1) / tdim;
    int ye = static_cast<int>(ty) + (height - yp - 1) / tdim;

    // build a rect
    m_tilesRect = QRect(xs, ys, xe - xs + 1, ye - ys + 1);

    if (m_url.isEmpty())
        fetchTiles();

    emit invalidate(m_tilesRect);

    emit updated(QRect(0, 0, width, height));
}

void SlippyMap::render(QPainter *p, const QRect &rect) {
    for (int x = 0; x <= m_tilesRect.width(); ++x) {
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp(x + m_tilesRect.left(), y + m_tilesRect.top());
            QRect box = tileRect(tp);
            if (rect.intersects(box)) {
                if (m_tilePixmaps.contains(tp))
                    p->drawPixmap(box, m_tilePixmaps.value(tp));
                else
                    p->drawPixmap(box, m_emptyTile);
            }
        }
    }
    /* TODO2 this assumes that all overlays only draw icons of size 32x32 */
    QRect iconRect = rect.adjusted(-16, -16, 16, 16);
    for (int x = 0; x <= m_tilesRect.width(); ++x) {
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp(x + m_tilesRect.left(), y + m_tilesRect.top());
            QRect box = tileRect(tp);
            if (iconRect.intersects(box))
                emit tileRendered(p, tp, box);
        }
    }
}

void SlippyMap::pan(const QPoint &delta) {
    QPointF dx = QPointF(delta) / qreal(tdim);
    QPointF center = tileForCoordinate(latitude, longitude, zoom) - dx;
    latitude = latitudeFromTile(center.y(), zoom);
    longitude = longitudeFromTile(center.x(), zoom);
    invalidate();
}

QPoint SlippyMap::scrollOffset() const {
    return m_tilesRect.topLeft() * tdim - m_offset;
}

void SlippyMap::setScrollOffset(const QPoint &offset) {
    pan(-(offset - scrollOffset()));
}

void SlippyMap::mouseClicked(const QPoint &pos) {
    QPointF posf(pos + scrollOffset());
    QPointF tile = posf / tdim;
    emit mouseClicked(QPoint(floor(tile.x()), floor(tile.y())), pos);
}


void SlippyMap::handleNetworkData(QNetworkReply *reply) {
    QImage img;
    QPoint tp = reply->request().attribute(QNetworkRequest::User).toPoint();
    QUrl url = reply->url();
    if (!reply->error())
        if (!img.load(reply, 0))
            img = QImage();
    reply->deleteLater();
    m_tilePixmaps[tp] = QPixmap::fromImage(img);
    if (img.isNull())
        m_tilePixmaps[tp] = m_emptyTile;
    emit updated(tileRect(tp));

    // purge unused spaces
    QRect bound = m_tilesRect.adjusted(-2, -2, 2, 2);
    foreach(QPoint tp, m_tilePixmaps.keys())
        if (!bound.contains(tp))
            m_tilePixmaps.remove(tp);

    fetchTiles();
}

void SlippyMap::fetchTiles() {
    QPoint grab(-1, -1);
    for (int x = 0; x <= m_tilesRect.width(); ++x)
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
        QPoint tp = m_tilesRect.topLeft() + QPoint(x, y);
        if (!m_tilePixmaps.contains(tp)) {
            grab = tp;
            break;
        }
    }
    if (grab == QPoint(-1, -1)) {
        m_url = QUrl();
        return;
    }
    /* TODO0 also use other offline map repositories in use on this device */

    QString path = "http://tile.openstreetmap.org/%1/%2/%3.png";
    m_url = QUrl(path.arg(zoom).arg(grab.x()).arg(grab.y()));
    if (!internetConnectionActive() && !m_manager.cache()->metaData(m_url).isValid()) {
        m_url = QUrl();
        return;
    }

    QNetworkRequest request;
    request.setUrl(m_url);
    request.setRawHeader("User-Agent", "Nokia (Qt) Graphics Dojo 1.0");
    request.setAttribute(QNetworkRequest::User, QVariant(grab));
    m_manager.get(request);
}

QRect SlippyMap::tileRect(const QPoint &tp) {
    QPoint t = tp - m_tilesRect.topLeft();
    int x = t.x() * tdim + m_offset.x();
    int y = t.y() * tdim + m_offset.y();
    return QRect(x, y, tdim, tdim);
}

QPoint SlippyMap::coordinateToPixels(const QPointF &c) const
{
    return ((tileForCoordinate(c.y(), c.x(), zoom) - m_tilesRect.topLeft()) * tdim + m_offset).toPoint();
}

ArticleOverlay::ArticleOverlay(Evopedia *evopedia, SlippyMap *parent)
    : QObject(parent),
    enabled(true),
    wikipediaIcon(":/map_icons/wikipedia.png"),
    evopedia(evopedia),
    slippyMap(parent)

{
    connect(parent, SIGNAL(invalidate(QRect)), SLOT(invalidate(QRect)));
    connect(parent, SIGNAL(tileRendered(QPainter*,QPoint,QRect)), SLOT(tileRendered(QPainter*,QPoint,QRect)));
    connect(parent, SIGNAL(mouseClicked(QPoint,QPoint)), SLOT(mouseClicked(QPoint,QPoint)));
}

QList<GeoTitle> ArticleOverlay::getTitles(const QRectF &rect, int maxTitles)
{
    QList<GeoTitle> list;
    /* TODO2 fair division between languages? */
    foreach (StorageBackend *b, evopedia->getBackends()) {
        list += b->getTitlesInCoords(rect, maxTitles - list.length());
        if (list.length() >= maxTitles)
            return list;
    }
    return list;
}

void ArticleOverlay::invalidate(const QRect &tilesRect)
{
    for (int x = 0; x <= tilesRect.width(); ++x) {
        for (int y = 0; y <= tilesRect.height(); ++y) {
            QPoint tile(tilesRect.left() + x, tilesRect.top() + y);
            if (titles.contains(tile))
                continue;
            titles[tile] = getTitles(coordinatesFromTile(tile, slippyMap->zoom), 10);
            /* TODO1: "zoom in for more articles" */
        }
    }
    /* TODO0 keep hash small by removing members that have not been used recently */
}

void ArticleOverlay::tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox)
{
    if (!enabled || !titles.contains(tile))
        return;

    foreach (GeoTitle t, titles[tile]) {
        QPoint pos = slippyMap->coordinateToPixels(t.getCoordinate());
        QSize halfSize(wikipediaIcon.size() / 2);
        pos -= QPoint(halfSize.width(), halfSize.height());
        p->drawPixmap(pos, wikipediaIcon);
    }
}

void ArticleOverlay::mouseClicked(const QPoint &tile, const QPoint &pixelPos)
{
    if (!titles.contains(tile))
        return;

    foreach (GeoTitle t, titles[tile]) {
        QPoint pos = slippyMap->coordinateToPixels(t.getCoordinate());
        QPoint delta = pos - pixelPos;
        /* TODO1 use the nearest article */
        if (abs(delta.x()) < 20 && abs(delta.y()) < 20) {
            titleSelected(t);
            return;
        }
    }
}

void ArticleOverlay::titleSelected(const GeoTitle &t)
{
    Title title = t.getTitle();
    QMessageBox msgBox(QMessageBox::NoIcon, "Article", title.getReadableName(),
                        QMessageBox::Open | QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Open) {
        QDesktopServices::openUrl(evopedia->getArticleUrl(title));
    }
}
