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

#include "mapwindow.h"
#include "ui_mapwindow.h"

#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#include <QMessageBox>
#include <QTimer>

MapWindow::MapWindow(QWidget *parent) :
    QMainWindow(parent),
#if defined(USE_MOBILITY)
    posSource(QGeoPositionInfoSource::createDefaultSource(this)),
#endif
    ui(new Ui::MapWindow)
{
    ui->setupUi(this);
    ui->mapWidget->setFocus();
#if defined(USE_MOBILITY)
    if (posSource != 0)
        connect(posSource, SIGNAL(positionUpdated(QGeoPositionInfo)), SLOT(positionUpdated(QGeoPositionInfo)));
#else
    ui->actionGo_to_GPS_Position->setEnabled(false);
    ui->actionUse_GPS->setEnabled(false);
    ui->actionFollow_GPS->setEnabled(false);
#endif

#if defined(Q_OS_SYMBIAN)
    QAction *closeAction = new QAction(tr("Back"), ui->menuMenu);
    connect(closeAction, SIGNAL(triggered()), SLOT(close()));
    ui->menuMenu->addAction(closeAction);
#endif

    connect(ui->actionShow_Articles, SIGNAL(toggled(bool)), ui->mapWidget, SLOT(overlaysEnable(bool)));
    connect(ui->actionZoom_In, SIGNAL(triggered()), ui->mapWidget, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), ui->mapWidget, SLOT(zoomOut()));

    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

MapWindow::~MapWindow()
{
    delete ui;
}

#ifdef Q_WS_MAEMO_5
void MapWindow::grabZoomKeys(bool grab)
{
    if (!winId()) {
        qWarning("Can't grab keys unless we have a window id");
        return;
    }

    unsigned long val = (grab) ? 1 : 0;
    Atom atom = XInternAtom(QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", False);
    if (!atom) {
        qWarning("Unable to obtain _HILDON_ZOOM_KEY_ATOM. This example will only work "
                 "on a Maemo 5 device!");
        return;
    }

    XChangeProperty (QX11Info::display(),
                     winId(),
                     atom,
                     XA_INTEGER,
                     32,
                     PropModeReplace,
                     reinterpret_cast<unsigned char *>(&val),
                     1);
}
#endif


void MapWindow::setPosition(qreal lat, qreal lon, int zoom)
{
    ui->mapWidget->setPosition(lat, lon, zoom);
}

void MapWindow::getPosition(qreal &lat, qreal &lng, int &zoom)
{
    ui->mapWidget->getPosition(lat, lng, zoom);
}

#if defined(USE_MOBILITY)
void MapWindow::positionUpdated(const QGeoPositionInfo &posInfo)
{
    const QGeoCoordinate coordinate = posInfo.coordinate();
    if (ui->actionFollow_GPS->isChecked() && coordinate.isValid()) {
        setPosition(coordinate.latitude(), coordinate.longitude());
    }
}
#endif

void MapWindow::on_actionUse_GPS_toggled(bool value)
{
#if defined(USE_MOBILITY)
    if (posSource == 0) {
        QMessageBox::critical(this, tr("Error"), tr("No position info source available."));
        return;
    }

    if (value)
        posSource->startUpdates();
    else
        posSource->stopUpdates();
    ui->actionFollow_GPS->setEnabled(value);
    ui->actionGo_to_GPS_Position->setEnabled(value);
#else
    Q_UNUSED(value);
#endif
}

void MapWindow::on_actionGo_to_GPS_Position_triggered()
{
#if defined(USE_MOBILITY)
    if (posSource == 0) return;
    const QGeoCoordinate coordinate = posSource->lastKnownPosition().coordinate();
    if (coordinate.isValid())
        setPosition(coordinate.latitude(), coordinate.longitude());
#endif
}

void MapWindow::on_actionFollow_GPS_toggled(bool value)
{
    if (value)
        on_actionGo_to_GPS_Position_triggered();
}
