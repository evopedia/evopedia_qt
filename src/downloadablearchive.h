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

#ifndef DOWNLOADABLEARCHIVE_H
#define DOWNLOADABLEARCHIVE_H

#include <QUrl>
#include <QNetworkReply>

#include "archive.h"


class DownloadableArchive : public Archive
{
    Q_OBJECT

    QUrl url;
    QString size;

    /* only used and initialized after download started */
    QString downloadDirectory;
    QString torrentFile;

    QString askAndCreateDownloadDirectory();

private slots:
    void torrentDownloadFinished(QNetworkReply* reply);

public:
    explicit DownloadableArchive(const QString &language, const QString &date,
                                 const QUrl &url, const QString &size, QObject *parent = 0);
    const QString &getSize() const { return size; }

signals:

public slots:

    bool startDownload();

};

#endif // DOWNLOADABLEARCHIVE_H
