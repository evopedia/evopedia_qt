#ifndef ARCHIVEITEM_H
#define ARCHIVEITEM_H

#include <QStandardItem>
#include <QString>
#include <QUrl>
#include "storagefrontend.h"
#include "torrentfrontend.h"

class QMenu;
class StorageBackend;

namespace ItemState {
    enum {Local, LocalTorrent, RemoteTorrent};
}

class ArchiveItem : public QStandardItem {
  friend class ArchiveManager;
  friend class StorageFrontend;
  friend class TorrentFrontend;

protected:
    ArchiveItem(QString language, QString date, QString dir, QString torrent, QUrl url);
    ArchiveItem(QString dir);
    ~ArchiveItem();
    StorageBackend* storageBackend();
    StorageFrontend* m_storagefrontend;
    TorrentFrontend* m_torrentfrontend;
    QString language();
    QString date();
    QString dir();
    QString size();
    QUrl url();
    QString state();
    int itemState();
    int type() const;
    void setStateString(QString state);
    void update();
    bool activated();
    void removeEntry();

public:
    QMenu* createContextMenu();
    bool validate(QString& ret);

private:
    QString m_size;
    QString m_language;
    QString m_date;
    QString m_torrent;
    QString m_dir;
    QUrl m_url;
    int m_itemState; // describes what kind of item we have, see namespace ItemState
    QString m_state;  // message for the user
};

#endif // ARCHIVEITEM_H
