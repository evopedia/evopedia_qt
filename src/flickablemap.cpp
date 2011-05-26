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

#include "flickablemap.h"

FlickableMap::FlickableMap(QWidget *parent) :
    QWidget(parent), articleOverlay(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);

    map = new SlippyMap(this);
    connect(map, SIGNAL(updated(QRect)), SLOT(updateMap(QRect)));

    articleOverlay = new ArticleOverlay(map);

    setMouseTracking(true);
    Flickable::setAcceptMouseClick(this);
}

void FlickableMap::setPosition(qreal lat, qreal lng, int zoom)
{
    map->setPosition(lat, lng, zoom);
    externalScrollUpdate();
}

void FlickableMap::getPosition(qreal &lat, qreal &lng, int &zoom)
{
    map->getPosition(lat, lng, zoom);
}

void FlickableMap::updateMap(const QRect &r)
{
    update(r);
}

void FlickableMap::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    map->resize(width(), height());
    externalScrollUpdate();
}

void FlickableMap::paintEvent(QPaintEvent *event)
{
    QPainter p;
    p.begin(this);
    map->render(&p, event->rect());
    p.setPen(Qt::black);
#if defined(Q_OS_SYMBIAN)
    QFont font = p.font();
    font.setPixelSize(13);
    p.setFont(font);
#endif
    p.drawText(rect(),  Qt::AlignBottom | Qt::TextWordWrap,
               tr("Map data CCBYSA 2010 OpenStreetMap.org contributors"));
    if (!articleOverlay->isComplete() && articleOverlay->isEnabled()) {
        p.drawText(rect(), Qt::AlignTop | Qt::TextWordWrap,
                    tr("Zoom in for more articles"));
    }

    p.end();
}

void FlickableMap::mousePressEvent(QMouseEvent *event)
{
    Flickable::handleMousePress(event);
    if (event->isAccepted())
        return;

    if (event->button() == Qt::LeftButton) {
        map->mouseClicked(event->pos());
        event->accept();
    }
}

void FlickableMap::mouseMoveEvent(QMouseEvent *event)
{
    Flickable::handleMouseMove(event);
}

void FlickableMap::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        map->pan(QPoint(20, 0));
        break;
    case Qt::Key_Right:
        map->pan(QPoint(-20, 0));
        break;
    case Qt::Key_Up:
        map->pan(QPoint(0, 20));
        break;
    case Qt::Key_Down:
        map->pan(QPoint(0, -20));
        break;
    case Qt::Key_F7:
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_F8:
    case Qt::Key_Minus:
        zoomOut();
        break;
    default:
        return;
    }
    externalScrollUpdate();
}

void FlickableMap::zoomIn()
{
    map->setZoom(qBound(0, map->getZoom() + 1, 18));
    externalScrollUpdate();
}

void FlickableMap::zoomOut()
{
    map->setZoom(qBound(0, map->getZoom() - 1, 18));
    externalScrollUpdate();
}

void FlickableMap::overlaysEnable(bool value)
{
    if (articleOverlay) {
        articleOverlay->setEnabled(value);
        updateMap(rect());
    }
}

void FlickableMap::mouseReleaseEvent(QMouseEvent *event)
{
    Flickable::handleMouseRelease(event);
    if (event->isAccepted())
        return;
}

void FlickableMap::wheelEvent(QWheelEvent *event)
{
    if (event->orientation() == Qt::Horizontal) {
        event->ignore();
    } else {
        if (event->delta() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    }
}

void FlickableMap::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    map->show();
}

void FlickableMap::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    map->hide();
}
