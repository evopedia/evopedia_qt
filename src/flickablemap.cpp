#include "flickablemap.h"

FlickableMap::FlickableMap(QWidget *parent) :
    QWidget(parent), articleOverlay(0)
{
    map = new SlippyMap(this);
    connect(map, SIGNAL(updated(QRect)), SLOT(updateMap(QRect)));

    articleOverlay = new ArticleOverlay(map);

    setMouseTracking(true);
    Flickable::setAcceptMouseClick(this);
}

void FlickableMap::setPosition(qreal lat, qreal lng, int zoom)
{
    map->latitude = lat;
    map->longitude = lng;
    if (zoom > 0) {
        map->zoom = qBound(2, zoom, 18);
    }
    map->invalidate();
    externalScrollUpdate();
}

void FlickableMap::getPosition(qreal &lat, qreal &lng, int &zoom)
{
    lat = map->latitude;
    lng = map->longitude;
    zoom = map->zoom;
}

void FlickableMap::updateMap(const QRect &r)
{
    update(r);
}

void FlickableMap::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    map->width = width();
    map->height = height();
    map->invalidate();
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
    map->zoom  = qBound(2, map->zoom + 1, 18);
    map->invalidate();
}

void FlickableMap::zoomOut()
{
    map->zoom  = qBound(2, map->zoom - 1, 18);
    map->invalidate();
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

void FlickableMap::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    map->visible = true;
    map->invalidate();
}

void FlickableMap::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    map->visible = false;
}
