#ifndef ARCHIVEITEM_H
#define ARCHIVEITEM_H

#include <QStandardItem>
#include <QString>
#include <QUrl>
#include "storagefrontend.h"
#include "torrentfrontend.h"

class QMenu;
class LocalArchive;

namespace ItemState {
    enum {Local, LocalTorrent, DownloadingTorrent, DownloadableArchive};
}

/*

 Actions in UI:
 start/stop torrent, update information in torrent, remove partial download (?)


 */

class ArchiveListItem : public QStandardItem {
  friend class ArchiveManager;

protected:
    ArchiveListItem(QString language, QString date, QString size, QString workingDir, QString archiveDir, QString torrent, QUrl url);
    ArchiveListItem(QString archiveDir);
    ~ArchiveListItem();
    LocalArchive* storageBackend();
    StorageFrontend* m_storagefrontend;
    TorrentFrontend* m_torrentfrontend;
    QString language();
    QString date();
    QString dir();
    QString size();
    QUrl url();
    QString stateString();
    bool activated();
    int itemState();
    int type() const;

public:
    QMenu* createContextMenu();
    bool validate(QString& ret);
    void removeEntry();
    void update();
    void setItemState(int s);

private:
    int m_itemState; // describes what kind of item we have, see namespace ItemState
};



#endif // ARCHIVEITEM_H
