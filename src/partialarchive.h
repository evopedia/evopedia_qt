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

#ifndef PARTIALARCHIVE_H
#define PARTIALARCHIVE_H

#include <QSettings>

#include "archive.h"
#include "torrent/torrentclient.h"


class PartialArchive : public Archive
{
    Q_OBJECT

    QString size;
    QString torrentFile;
    QString dir;

    float uploadRate;
    float downloadRate;

    TorrentClient* torrentClient;

    void changeToLocalArchive();
    static QString getStateText(TorrentClient::State s);

private slots:
    void updateState(TorrentClient::State s);
    void updatePeerInfo();
    void updateDownloadRate(int rate);
    void updateUploadRate(int rate);
    void torrentStopped();
    void torrentError(TorrentClient::Error);

public:
    PartialArchive(const QString &language, const QString &date,
                                 const QString &size,
                                 const QString &torrentFile, const QString &dir,
                                 QObject *parent = 0);
    static PartialArchive *restoreArchive(QSettings &settings, QObject *parent = 0);

    void saveToSettings(QSettings &settings) const;


    void setExternallyPaused(bool value);
    bool isDownloading() const;
    QString getSizeMB() const;

    void emitStatusEvents();

    /* TODO1 torrent based chunk validation */
    bool validate(QString &ret);

signals:
    void progressUpdated(int percent);
    void peerInfoUpdated(const QString &peerInfo);
    void speedTextUpdated(const QString &speedText);
    void statusTextUpdated(const QString &statusText);

    void downloadPaused();
    void downloadStarted();

public slots:

    void startDownload();
    void pauseDownload();
    void togglePauseDownload();
};

#endif // PARTIALARCHIVE_H
