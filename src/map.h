#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QPoint>
#include <QPointF>
#include <QPainter>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "evopedia.h"
#include "flickable.h"


class SlippyMap: public QObject
{
    Q_OBJECT

public:
    int width;
    int height;
    int zoom;
    qreal latitude;
    qreal longitude;

    SlippyMap(QObject *parent = 0);
    void invalidate();
    void render(QPainter *p, const QRect &rect);
    void pan(const QPoint &delta);
    QPoint coordinateToPixels(const QPointF &c) const;
    QPoint scrollOffset() const;
    void setScrollOffset(const QPoint &offset);

    void mouseClicked(const QPoint &pos);

private slots:

    void handleNetworkData(QNetworkReply *reply);
    void fetchTiles();
signals:
    void updated(const QRect &rect);
    void invalidate(const QRect &tilesRect);
    void tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox);
    void mouseClicked(const QPoint &tile, const QPoint &pixelPos);

protected:
    QRect tileRect(const QPoint &tp);

private:
    QPoint m_offset;
    QRect m_tilesRect;
    QPixmap m_emptyTile;
    QHash<QPoint, QPixmap> m_tilePixmaps;
    QNetworkAccessManager m_manager;
    QUrl m_url;
};

class ArticleOverlay: public QObject
{
    Q_OBJECT

public:
    ArticleOverlay(Evopedia *evopedia, SlippyMap *parent);

public slots:
    void setEnabled(bool value) { enabled = value; }

private slots:
    void invalidate(const QRect &tilesRect);
    void tileRendered(QPainter *p, const QPoint &tile, const QRect drawBox);
    void mouseClicked(const QPoint &tile, const QPoint &pixelPos);

private:
    QList<GeoTitle> getTitles(const QRectF &rect, int maxTitles);
    void titleSelected(const GeoTitle &t);

    bool enabled;
    QHash<QPoint, QList<GeoTitle> > titles;
    QPixmap wikipediaIcon;
    Evopedia *evopedia;
    SlippyMap *slippyMap;
};

#endif // MAP_H
