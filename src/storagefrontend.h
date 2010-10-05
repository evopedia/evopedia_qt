#ifndef STORAGEFRONTEND_H
#define STORAGEFRONTEND_H

#include "abstractfrontend.h"

class QMenu;
class ArchiveItem;

class StorageFrontend : public AbstractFrontend {
    Q_OBJECT
    friend class ArchiveItem;
protected:
    StorageFrontend(ArchiveItem* item);
    ~StorageFrontend();
    QMenu* createContextMenu();
private:
    ArchiveItem* m_archiveitem;
Q_SIGNALS:
    void updateBackends();
public Q_SLOTS:
    void removeEntry();
};

#endif // STORAGEFRONTEND_H
