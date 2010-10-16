#ifndef PARTIALARCHIVE_H
#define PARTIALARCHIVE_H

#include <QUrl>

#include "archive.h"
#include "torrent/torrentclient.h"


class PartialArchive : public Archive
{
    Q_OBJECT

    QUrl url;
    QString size;
    QString torrentFile;
    QString dir;

    int uploadRate;
    int downloadRate;

    TorrentClient* torrentClient;

public:
    explicit PartialArchive(const QString &language, const QString &date,
                                 const QUrl &url, const QString &size,
                                 const QString &torrentFile, const QString &dir,
                                 QObject *parent = 0);



    /*! torrent based chunk validation */
    bool validate(QString &ret);

signals:
    void progressUpdated(int percent);
    void peerInfoUpdated(QString peerInfo);
    void speedTextUpdated(QString speedText);

public slots:

    void startDownload();
    void pauseDownload();
    void cancelDownload();

    void updateState(TorrentClient::State s);
    void updatePeerInfo();
    void updateDownloadRate(int rate);
    void updateUploadRate(int rate);
    void torrentStopped();
    void torrentError(TorrentClient::Error);
};

#endif // PARTIALARCHIVE_H
