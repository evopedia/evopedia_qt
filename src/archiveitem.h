#ifndef ARCHIVEITEM_H
#define ARCHIVEITEM_H

#include <QStandardItem>
#include <QUrl>
#include <QString>
#include "storagebackend.h"
#include "storagefrontend.h"

class QMenu;

namespace ItemState {
    enum {Local, RemoteTorrent, LocalTorrent};
}

class ArchiveItem : protected QStandardItem {
  friend class ArchiveManager;

protected:
    ArchiveItem(QString language, QString date, QString dir, QString torrent, QUrl url);
    ArchiveItem(QString dir);
    ~ArchiveItem();
    void extend();
    void changeBackend(int type);
    bool validate(QString &ret);
    StorageFrontend* muffin;
    QString language();
    QString date();
    QString dir();
    QString size();
    int itemState();
    QUrl url();
    QString state();
    int type() const;
    StorageBackend* storageBackend();
    void setState(QString state);
    void update();
    void store();
    bool activated();
public:
    QMenu* createContextMenu();

private:
    int m_itemState; // describes what kind of item we have, see namespace ItemState
    QString m_size;
    QString m_language;
    QString m_date;
    QString m_dir;
    QUrl m_url;
    QString m_state; // message for the user
    bool m_activated; // is item in use?
    StorageBackend *m_storageBackend;
//Q_SIGNALS:
  //  void updateBackends();
};

#endif // ARCHIVEITEM_H
