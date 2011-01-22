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

#include "tilefetcher.h"

#include <QMutexLocker>

#include "defines.h"
#include "evopedia.h"
#include "evopediaapplication.h"

TileFetcher::TileFetcher(QObject *parent) :
    QThread(parent)
{
}

void TileFetcher::run()
{
    netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*)), SLOT(networkRequestCompleted(QNetworkReply*)));

    exec();
}

void TileFetcher::fetchTile(int zoom, QPoint offset)
{
    Q_ASSERT(0 <= offset.x() && offset.x() < (1 << zoom));
    Q_ASSERT(0 <= offset.y() && offset.y() < (1 << zoom));

    QString path = "http://tile.openstreetmap.org/%1/%2/%3.png";
    QUrl url(path.arg(zoom).arg(offset.x()).arg(offset.y()));

    /* TODO1 thread safe? */
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    if (!evopedia->networkConnectionAllowed() /* TODO1 && !m_manager.cache()->metaData(m_url).isValid()*/)
        return;

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "Nokia (Qt) Graphics Dojo 1.0");
    request.setAttribute(QNetworkRequest::User, QVariant(zoom));
    request.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1), QVariant(offset));
    netManager->get(request);
}

void TileFetcher::networkRequestCompleted(QNetworkReply *reply)
{
    QImage img;
    int zoom = reply->request().attribute(QNetworkRequest::User).toInt();
    QPoint offset = reply->request().attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1)).toPoint();

    reply->deleteLater();
    if (reply->error() || !img.load(reply, "PNG"))
        return;

    /* TODO1 on maemo only if MyDocs is mounted! */
    QString path = QString("%1/%2/%3/%4")
                   .arg(MAPTILES_LOCATION)
                   .arg("OpenStreetMap I")
                   .arg(zoom)
                   .arg(offset.x());
    QDir().mkpath(path);
    img.save(QString("%1/%2.png").arg(path).arg(offset.y()));

    emit tileLoaded(zoom, offset, img);
}
