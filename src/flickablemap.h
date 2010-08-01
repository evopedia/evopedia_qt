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
    void setEvopedia(Evopedia *evopedia);
    void setPosition(qreal lat, qreal lng, int zoom=-1);

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
    void keyPressEvent(QKeyEvent *event);

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
