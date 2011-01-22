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

#ifndef TILEFETCHER_H
#define TILEFETCHER_H

#include <QObject>
#include <QThread>
#include <QPoint>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>

class TileFetcher : public QThread
{
    Q_OBJECT

    QNetworkAccessManager *netManager;

private slots:
    void networkRequestCompleted(QNetworkReply *reply);

protected:
    void run();

public:
    explicit TileFetcher(QObject *parent = 0);

signals:
    void tileLoaded(int zoom, QPoint offset, QImage pixmap);

public slots:
    void fetchTile(int zoom, QPoint offset);

};

#endif // TILEFETCHER_H
