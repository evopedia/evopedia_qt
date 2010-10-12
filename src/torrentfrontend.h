#ifndef TORRENTFRONTEND_H
#define TORRENTFRONTEND_H

#include <QUrl>
#include "abstractfrontend.h"
#include "torrent/torrentclient.h"

class QMenu;
class ArchiveItem;
class QNetworkReply;

class TorrentFrontend : public AbstractFrontend {
    Q_OBJECT
public:
    TorrentFrontend(ArchiveItem* item, QString language, QString date, QString size, QString workingDir, QString archiveDir, QString torrent, QUrl url);
    QMenu* createContextMenu();
    void saveSettings();
    void unsaveSettings();
    bool validate(QString &ret);
    QString language();
    QString date();
    QString workingDir();
    QString archiveDir();
    QString size();
    QUrl url();
    QString stateString();
private:
    ArchiveItem* m_archiveitem;
    TorrentClient* m_torrentclient;
    QString m_size;
    QString m_language;
    QString m_date;
    QString m_torrent;
    QString m_workingDir;
    QString m_archiveDir;
    QString m_state;
    QUrl m_url;
private Q_SLOTS:
    void extend();
    void collapse();
    void startTorrentDownload();
    void startDownloadViaTorrent();
    void torrentDownloadFinished(QNetworkReply* reply);
    void resumeTorrentDownload();
    void pauseTorrentDownload();
    void cancelTorrentDownload();

    void updateState(TorrentClient::State);
    void updatePeerInfo();
    void updateProgress(int);
    void updateDownloadRate(int);
    void updateUploadRate(int);
    void torrentStopped();
    void torrentError(TorrentClient::Error);
};

#endif // TORRENTFRONTEND_H
