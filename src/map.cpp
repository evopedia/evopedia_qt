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
#include <QtAlgorithms>

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif

#if defined (Q_OS_SYMBIAN)
//#include "sym_iap_util.h"
#endif

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "evopediaapplication.h"
#include "map.h"
#include "utils.h"
#include "localarchive.h"

inline uint qHash(const QPoint& p)
{
    return p.x() * 17 ^ p.y();
}

inline uint qHash(const ArticleOverlay::ZoomTile &key)
{
    return qHash(key.tile) ^ (19 * qHash(key.zoom));
}


// tile size in pixels
const int tdim = 256;


SlippyMap::SlippyMap(QObject *parent)
    : QObject(parent),
    width(400),
    height(300),
    zoom(15),
    m_centerPos(16384, 10900)
{
    m_emptyTile = QPixmap(tdim, tdim);
    m_emptyTile.fill(Qt::lightGray);

    /* disabled because default cache location is on root fs
    QNetworkDiskCache *cache = new QNetworkDiskCache;
    cache->setCacheDirectory(QDesktopServices::storageLocation
                             (QDesktopServices::CacheLocation));
    m_manager.setCache(cache);
    */

    tileFetcher = new TileFetcher();
    tileFetcher->start(QThread::LowPriority);
    tileFetcher->moveToThread(tileFetcher);
    connect(tileFetcher, SIGNAL(tileLoaded(int,QPoint,QImage)), SLOT(tileLoaded(int,QPoint,QImage)));
    connect(this, SIGNAL(tileNeeded(int,QPoint)), tileFetcher, SLOT(fetchTile(int,QPoint)));
}

void SlippyMap::invalidate()
{
    if (width <= 0 || height <= 0 || !visible)
        return;

    boundPosition();

    qreal tx = m_centerPos.x();
    qreal ty = m_centerPos.y();

    // top-left corner of the center tile
    int xp = width / 2 - (tx - floor(tx)) * tdim;
    int yp = height / 2 - (ty - floor(ty)) * tdim;

    // first tile vertical and horizontal
    int xa = (xp + tdim - 1) / tdim;
    int ya = (yp + tdim - 1) / tdim;
    int xs = static_cast<int>(floor(tx)) - xa;
    int ys = static_cast<int>(floor(ty)) - ya;

    m_topLeftOffset = QPoint(xp - xa * tdim, yp - ya * tdim);

    // last tile vertical and horizontal
    int xe = static_cast<int>(tx) + (width - xp - 1) / tdim;
    int ye = static_cast<int>(ty) + (height - yp - 1) / tdim;

    m_tilesRect = QRect(xs, ys, xe - xs + 1, ye - ys + 1);

    fetchTiles();

    emit invalidate(m_tilesRect);

    emit updated(QRect(0, 0, width, height));
}

void SlippyMap::render(QPainter *p, const QRect &rect)
{
    for (int x = 0; x <= m_tilesRect.width(); ++x) {
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp(x + m_tilesRect.left(), y + m_tilesRect.top());
            QRect box = tileRect(tp);
            if (rect.intersects(box)) {
                tp.setX(tp.x() & ((1 << zoom) - 1));
                if (m_tilePixmaps.contains(tp) && !m_tilePixmaps[tp].isNull())
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
            if (iconRect.intersects(box)) {
                tp.setX(tp.x() & ((1 << zoom) - 1));
                emit tileRendered(p, tp, box);
            }
        }
    }
}

void SlippyMap::pan(const QPoint &delta)
{
    m_centerPos -= QPointF(delta) / qreal(tdim);
    invalidate();
}

QPoint SlippyMap::scrollOffset() const
{
    return (m_centerPos * tdim).toPoint();
}

void SlippyMap::setScrollOffset(const QPoint &offset)
{
    pan(-(offset - scrollOffset()));
}

void SlippyMap::mouseClicked(const QPoint &pos)
{
    QPointF posf(pos + scrollOffset());
    QPointF tile = posf / tdim;
    emit mouseClicked(QPoint(floor(tile.x()), floor(tile.y())), pos);
}


void SlippyMap::tileLoaded(int z, QPoint offset, QImage image)
{
    if (z != this->zoom)
        return;

    m_tilePixmaps[offset] = QPixmap::fromImage(image);
    /* XXX enough? pixmap could be visible multiple times (zoom 1) */
    emit updated(tileRect(offset));

    // purge unused spaces
    /* TODO better: have some fixed size cache and remove the tiles
     * that were used least recently (also store tiles of different
     * zoom levels) */
    QRect bound = m_tilesRect.adjusted(-3, -3, 3, 3);
    foreach(QPoint tp, m_tilePixmaps.keys())
        if (!bound.contains(tp))
            m_tilePixmaps.remove(tp);
}

void SlippyMap::fetchTiles()
{
    /* TODO "refresh" m_tilePixmaps once we get online (i.e. remove all the null tiles) */
    for (int x = m_tilesRect.left(); x <= m_tilesRect.right(); ++x) {
        for (int y = m_tilesRect.top(); y <= m_tilesRect.bottom(); ++y) {
            if (y >= 0 && y < (1 << zoom)) {
                QPoint tp = QPoint(x & ((1 << zoom) - 1), y);
                if (!m_tilePixmaps.contains(tp)) {
                    m_tilePixmaps[tp] = QPixmap();
                    emit tileNeeded(zoom, tp);
                }
            }
        }
    }
}

void SlippyMap::boundPosition()
{
    if ((1 << zoom) * tdim < height) {
        m_centerPos.setY((1 << zoom) / 2.0);
    } else {
        qreal y = qBound<qreal>(height / 2.0, m_centerPos.y() * tdim, (1 << zoom) * tdim - height / 2);
        m_centerPos.setY(y / tdim);
    }
}

QRect SlippyMap::tileRect(const QPoint &tp) {
    QPoint t = tp - m_tilesRect.topLeft();
    int x = t.x() * tdim + m_topLeftOffset.x();
    int y = t.y() * tdim + m_topLeftOffset.y();
    return QRect(x, y, tdim, tdim);
}

QPoint SlippyMap::titleCoordinateToPixels(const QPointF &c) const
{
    return ((project(c, zoom) - m_centerPos) * tdim).toPoint() + QPoint(width / 2, height / 2);
}

void SlippyMap::setZoom(int z)
{
    if (z > this->zoom)
        m_centerPos *= 1 << (z - this->zoom);
    else
        m_centerPos /= 1 << (this->zoom - z);
    this->zoom = z;

    m_tilePixmaps.clear();

    invalidate();
}

void SlippyMap::getPosition(qreal &lat, qreal &lng, int &zoom)
{
    QPointF c(unproject(m_centerPos, this->zoom));
    lat = c.y();
    lng = c.x();
    zoom = this->zoom;
}

void SlippyMap::setPosition(qreal lat, qreal lng, int zoom)
{
    m_centerPos = project(QPointF(lng, lat), zoom);
    this->zoom = zoom;

    invalidate();
}

QPointF SlippyMap::unproject(const QPointF &xy, int zoom)
{
    qreal x = xy.x();
    qreal y = xy.y();
    if (zoom > 0) {
        x /= qreal(1 << zoom);
        y /= qreal(1 << zoom);
    }

    while (x < 0.0) x += 1.0;
    while (x > 1.0) x -= 1.0;
    if (y < 0.0) y = 0;
    if (y > 1.0) y = 1.0;

    qreal n = M_PI - 2 * M_PI * y;
    return QPointF(x * 360.0 - 180.0,
                   180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n))));
}

QRectF SlippyMap::unprojectTileRect(const QPoint &tile, int zoom)
{
    QPointF bottomLeft = unproject(tile, zoom);
    QPointF topRight = unproject(tile + QPoint(1, 1), zoom);
    return QRectF(bottomLeft, topRight);
}

QPointF SlippyMap::project(QPointF lnglat, int zoom)
{
    qreal lng = lnglat.x();
    qreal lat = lnglat.y();

    qreal x;
    qreal y;
    if (lat <= -90.0) {
        x = .5;
        y = 0;
    } else if (lat >= 90.0) {
        x = .5;
        y = 1.0;
    } else {
        x = (lng + 180.0) / 360.0;
        y = (1.0 - log(tan(lat * M_PI / 180.0) +
                          1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0;

        while (x < 0) x += 1.0;
        while (x > 1.0) x -= 1.0;
    }

    return QPointF(x * (1 << zoom), y * (1 << zoom));
}

ArticleOverlay::ArticleOverlay(SlippyMap *parent)
    : QObject(parent),
    enabled(true),
    wikipediaIcon(":/map_icons/wikipedia.png"),
    slippyMap(parent)

{
    connect(parent, SIGNAL(invalidate(QRect)), SLOT(invalidate(QRect)));
    connect(parent, SIGNAL(tileRendered(QPainter*,QPoint,QRect)), SLOT(tileRendered(QPainter*,QPoint,QRect)));
    connect(parent, SIGNAL(mouseClicked(QPoint,QPoint)), SLOT(mouseClicked(QPoint,QPoint)));
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    connect(evopedia->getArchiveManager(),
            SIGNAL(defaultLocalArchivesChanged(QList<LocalArchive*>)),
            SLOT(backendsChanged(const QList<LocalArchive*>)));
}

ArticleOverlay::GeoTitleList ArticleOverlay::getTitles(const QRectF &rect, int maxTitles)
{
    ArticleOverlay::GeoTitleList list;
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    list.complete = false;

    /* TODO2 fair division between languages? */
    foreach (LocalArchive *b, evopedia->getArchiveManager()->getDefaultLocalArchives()) {
        list.list += b->getTitlesInCoords(rect, maxTitles - list.list.length());
        if (list.list.length() >= maxTitles)
            return list;
    }
    list.complete = true;
    return list;
}

void ArticleOverlay::invalidate(const QRect &tilesRect)
{
    int zoom = slippyMap->getZoom();
    for (int x = tilesRect.left(); x <= tilesRect.right(); ++x) {
        for (int y = tilesRect.top(); y <= tilesRect.bottom(); ++y) {
            if (y >= 0 && y < (1 << zoom)) {
                QPoint tp = QPoint(x & ((1 << zoom) - 1), y);
                ZoomTile zt(tp, zoom);
                if (titles.contains(zt))
                    continue;
                /* TODO does this work if the tile is near to -180 degrees? */
                titles[zt] = getTitles(SlippyMap::unprojectTileRect(tp, zoom), 20);
            }
        }
    }
    /* TODO0 keep hash small by removing members that have not been used recently */
}

bool ArticleOverlay::isComplete()
{
    int zoom = slippyMap->getZoom();

    const QRect tilesRect(slippyMap->getTilesRect());
    for (int x = 0; x <= tilesRect.width(); ++x) {
        for (int y = 0; y <= tilesRect.height(); ++y) {
            QPoint tile(tilesRect.left() + x, tilesRect.top() + y);
            ZoomTile zt(tile, zoom);
            if (titles.contains(zt) && !titles[zt].complete)
                return false;
        }
    }
    return true;
}


void ArticleOverlay::tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox)
{
    Q_UNUSED(drawBox);
    ZoomTile ztile(tile, slippyMap->getZoom());
    if (!enabled || !titles.contains(ztile))
        return;

    foreach (GeoTitle t, titles[ztile].list) {
        QPoint pos = slippyMap->titleCoordinateToPixels(t.getCoordinate());
        QSize halfSize(wikipediaIcon.size() / 2);
        pos -= QPoint(halfSize.width(), halfSize.height());
        p->drawPixmap(pos, wikipediaIcon);
    }
}

void ArticleOverlay::mouseClicked(const QPoint &tile, const QPoint &pixelPos)
{
    typedef QPair<GeoTitle,float> GeoTitleDistance;

    /* TODO this does not always work */

    int zoom = slippyMap->getZoom();

    QList<GeoTitleDistance> titleDistances;
    for (int x = -1; x <= 1; x ++) {
        for (int y = -1; y <= 1; y ++) {
            ZoomTile zt(tile + QPoint(x, y), zoom);
            if (!titles.contains(zt)) continue;
            foreach (GeoTitle t, titles[zt].list) {
                QPoint pos = slippyMap->titleCoordinateToPixels(t.getCoordinate());
                QPoint delta = pos - pixelPos;
                float distSq = delta.x() * delta.x() + delta.y() * delta.y();
                if (distSq > 25 * 25) continue;
                titleDistances += GeoTitleDistance(t, distSq);
            }
        }
    }
    if (titleDistances.isEmpty()) return;

    qSort(titleDistances.begin(), titleDistances.end(), GeoTitle::nearerThan);

    QList<Title> titleList;
    foreach (GeoTitleDistance t, titleDistances) {
        titleList += t.first.getTitle();
    }

    showNearTitleList(titleList);
}

void ArticleOverlay::showNearTitleList(const QList<Title> &list)
{
    if (list.isEmpty()) return;

    if (list.length() == 1) {
        QMessageBox msgbox(QMessageBox::NoIcon, "Article", list[0].getReadableName(),
                           QMessageBox::Open | QMessageBox::Cancel);
        if (msgbox.exec() == QMessageBox::Open) {
            (static_cast<EvopediaApplication *>(qApp))->openArticle(list[0]);
        }
    } else {
        QDialog dialog;
        dialog.setWindowTitle(tr("Articles"));
        QListWidget titleList(&dialog);
        titleList.setSelectionMode(QAbstractItemView::SingleSelection);
        QHBoxLayout layout(&dialog);
        layout.addWidget(&titleList);
        QDialogButtonBox buttons(QDialogButtonBox::Open | QDialogButtonBox::Cancel, Qt::Vertical, &dialog);
        layout.addWidget(&buttons);
        connect(&buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(&buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));

        /* TODO1 Use "infinite list" and show every article (not only those stored in the titles hash) */

        for (int i = 0; i < list.length(); i ++) {
            QListWidgetItem *item = new QListWidgetItem(list[i].getReadableName(), &titleList);
            item->setData(Qt::UserRole, i);
            if (i == 0)
                titleList.setCurrentItem(item);
        }

        if (dialog.exec() == QDialog::Accepted) {
            QList<QListWidgetItem *> selItems = titleList.selectedItems();
            if (selItems.empty()) return;

            Title t = list[selItems[0]->data(Qt::UserRole).toInt()];
            (static_cast<EvopediaApplication *>(qApp))->openArticle(t);
        }
    }
}

void ArticleOverlay::backendsChanged(const QList<LocalArchive *>)
{
    titles.clear();
    slippyMap->invalidate();
}
