#ifndef TORRENTFRONTEND_H
#define TORRENTFRONTEND_H

#include <QUrl>
#include "abstractfrontend.h"
#include "torrent/torrentclient.h"

class QMenu;
class ArchiveItem;

class TorrentFrontend : public AbstractFrontend {
    Q_OBJECT
public:
    TorrentFrontend(ArchiveItem* item);
    QMenu* createContextMenu();
protected:
    void saveSettings();
    void unsaveSettings();
    bool validate(QString &ret);
    QUrl m_url;
private:
    ArchiveItem* m_archiveitem;
    TorrentClient* m_torrentclient;
private Q_SLOTS:
    void extend();
    void startTorrentDownload();
    void resumeTorrentDownload();
    void pauseTorrentDownload();
    void cancelTorrentDownload();
    void torrentDownloadFinished();

    void updateState(TorrentClient::State);
    void updatePeerInfo();
    void updateProgress(int);
    void updateDownloadRate(int);
    void updateUploadRate(int);
    void torrentStopped();
    void torrentError(TorrentClient::Error);

};

#endif // TORRENTFRONTEND_H
