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

#ifdef Q_WS_MAEMO_5
#define MAPTILES_LOCATION "/home/user/MyDocs/.maps"
#else
#define MAPTILES_LOCATION (QDir::homePath() + "/.cache/maps")
#endif
/* TODO symbian */


#include "evopediaapplication.h"
#include "map.h"
#include "utils.h"
#include "storagebackend.h"

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

    /* disabled because default cache location is on root fs
    QNetworkDiskCache *cache = new QNetworkDiskCache;
    cache->setCacheDirectory(QDesktopServices::storageLocation
                             (QDesktopServices::CacheLocation));
    m_manager.setCache(cache);
    */
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));
}

void SlippyMap::invalidate() {
    if (width <= 0 || height <= 0 || !visible)
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

    if (!img.isNull()) {
        /* TODO1 on maemo only if MyDocs is mounted! */
        QString path = QString("%1/%2/%3/%4")
                       .arg(MAPTILES_LOCATION)
                       .arg("OpenStreetMap I")
                       .arg(zoom)
                       .arg(tp.x());
        QDir().mkpath(path);
        img.save(QString("%1/%2.png")
                 .arg(path).arg(tp.y()));
    }

    // purge unused spaces
    QRect bound = m_tilesRect.adjusted(-2, -2, 2, 2);
    foreach(QPoint tp, m_tilePixmaps.keys())
        if (!bound.contains(tp))
            m_tilePixmaps.remove(tp);

    m_url = QUrl();
    fetchTiles();
}

void SlippyMap::fetchTiles() {
    QPoint grab(-1, -1);
    for (int x = 0; x <= m_tilesRect.width(); ++x)
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
        QPoint tp = m_tilesRect.topLeft() + QPoint(x, y);
        if (!m_tilePixmaps.contains(tp)) {
            QPixmap img(QString("%1/%2/%3/%4/%5.png")
                        .arg(MAPTILES_LOCATION)
                        .arg("OpenStreetMap I")
                        .arg(zoom)
                        .arg(tp.x())
                        .arg(tp.y()));
            m_tilePixmaps[tp] = img;
            if (!img.isNull()) {
                emit updated(tileRect(tp));
            }
        }

        if (m_tilePixmaps[tp].isNull())
            grab = tp;
    }
    if (!m_url.isEmpty()) return;

    if (grab == QPoint(-1, -1)) {
        m_url = QUrl();
        return;
    }

    QString path = "http://tile.openstreetmap.org/%1/%2/%3.png";
    m_url = QUrl(path.arg(zoom).arg(grab.x()).arg(grab.y()));

    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    if (!evopedia->networkConnectionAllowed() /* TODO && !m_manager.cache()->metaData(m_url).isValid()*/) {
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
    connect(evopedia->archivemanager, SIGNAL(backendsChanged(const QList<StorageBackend*>)), SLOT(backendsChanged(const QList<StorageBackend*>)));
}

ArticleOverlay::GeoTitleList ArticleOverlay::getTitles(const QRectF &rect, int maxTitles)
{
    ArticleOverlay::GeoTitleList list;
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    list.complete = false;

    /* TODO2 fair division between languages? */
    foreach (StorageBackend *b, evopedia->archivemanager->getBackends()) {
        list.list += b->getTitlesInCoords(rect, maxTitles - list.list.length());
        if (list.list.length() >= maxTitles)
            return list;
    }
    list.complete = true;
    return list;
}

void ArticleOverlay::invalidate(const QRect &tilesRect)
{
    for (int x = 0; x <= tilesRect.width(); ++x) {
        for (int y = 0; y <= tilesRect.height(); ++y) {
            QPoint tile(tilesRect.left() + x, tilesRect.top() + y);
            ZoomTile zt(tile, slippyMap->zoom);
            if (titles.contains(zt))
                continue;
            titles[zt] = getTitles(coordinatesFromTile(tile, slippyMap->zoom), 20);
        }
    }
    /* TODO0 keep hash small by removing members that have not been used recently */
}

bool ArticleOverlay::isComplete()
{
    const QRect tilesRect(slippyMap->getTilesRect());
    for (int x = 0; x <= tilesRect.width(); ++x) {
        for (int y = 0; y <= tilesRect.height(); ++y) {
            QPoint tile(tilesRect.left() + x, tilesRect.top() + y);
            ZoomTile zt(tile, slippyMap->zoom);
            if (titles.contains(zt) && !titles[zt].complete)
                return false;
        }
    }
    return true;
}


void ArticleOverlay::tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox)
{
    Q_UNUSED(drawBox);
    ZoomTile ztile(tile, slippyMap->zoom);
    if (!enabled || !titles.contains(ztile))
        return;

    foreach (GeoTitle t, titles[ztile].list) {
        QPoint pos = slippyMap->coordinateToPixels(t.getCoordinate());
        QSize halfSize(wikipediaIcon.size() / 2);
        pos -= QPoint(halfSize.width(), halfSize.height());
        p->drawPixmap(pos, wikipediaIcon);
    }
}

void ArticleOverlay::mouseClicked(const QPoint &tile, const QPoint &pixelPos)
{
    typedef QPair<GeoTitle,float> GeoTitleDistance;

    QList<GeoTitleDistance> titleDistances;
    for (int x = -1; x <= 1; x ++) {
        for (int y = -1; y <= 1; y ++) {
            ZoomTile zt(tile + QPoint(x, y), slippyMap->zoom);
            if (!titles.contains(zt)) continue;
            foreach (GeoTitle t, titles[zt].list) {
                QPoint pos = slippyMap->coordinateToPixels(t.getCoordinate());
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

void ArticleOverlay::backendsChanged(const QList<StorageBackend *>)
{
    titles.clear();
    slippyMap->invalidate();
}
