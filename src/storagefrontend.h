#ifndef STORAGEFRONTEND_H
#define STORAGEFRONTEND_H

#include "abstractfrontend.h"
#include "storagebackend.h"

class QMenu;
class QString;
class ArchiveItem;

class StorageFrontend : public AbstractFrontend {
    Q_OBJECT
    friend class ArchiveItem;
protected:
    StorageFrontend(ArchiveItem* item);
    ~StorageFrontend();
    QMenu* createContextMenu();
    void saveSettings();
    void unsaveSettings();
    bool validate(QString &ret);
    StorageBackend* storageBackend();
    bool m_activated;
private:
    ArchiveItem* m_archiveitem;
    StorageBackend *m_storageBackend;
Q_SIGNALS:
    void updateBackends();
public Q_SLOTS:
    void removeEntry();
};

#endif // STORAGEFRONTEND_H
