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
