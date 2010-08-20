#ifndef MAPWINDOW_H
#define MAPWINDOW_H

#include <QMainWindow>

#include "map.h"

namespace Ui {
    class MapWindow;
}

class MapWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MapWindow(QWidget *parent = 0);
    ~MapWindow();

    /* TODO1 request current GPS position */
    void setPosition(qreal lat, qreal lng, int zoom=-1);
    void getPosition(qreal &lat, qreal &lng, int &zoom);

private slots:
    void delayedInit() {
#if defined(Q_OS_SYMBIAN)
//        qt_SetDefaultIap();
#endif
#if defined(Q_WS_MAEMO_5)
        grabZoomKeys(true);
#endif
    }

private:
#ifdef Q_WS_MAEMO_5
    void grabZoomKeys(bool grab);
#endif

    Ui::MapWindow *ui;
};

#endif // MAPWINDOW_H
