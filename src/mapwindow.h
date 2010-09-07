#ifndef MAPWINDOW_H
#define MAPWINDOW_H

#include <QMainWindow>

#if defined(USE_MOBILITY)
#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
QTM_USE_NAMESPACE
#endif

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

public slots:
#if defined(USE_MOBILITY)
    void positionUpdated(const QGeoPositionInfo &posInfo);
#endif
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

#if defined(USE_MOBILITY)
    QGeoPositionInfoSource *posSource;
#endif

    Ui::MapWindow *ui;

private slots:
    void on_actionFollow_GPS_toggled(bool );
    void on_actionUse_GPS_toggled(bool );
    void on_actionGo_to_GPS_Position_triggered();
};

#endif // MAPWINDOW_H
