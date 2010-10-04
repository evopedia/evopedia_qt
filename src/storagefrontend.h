#ifndef STORAGEFRONTEND_H
#define STORAGEFRONTEND_H

#include "abstractfrontend.h"

class QMenu;

class StorageFrontend : public AbstractFrontend {
    Q_OBJECT
    friend class ArchiveItem;
protected:
    StorageFrontend();
    ~StorageFrontend();
    QMenu* createContextMenu();
Q_SIGNALS:
    void updateBackends();
public Q_SLOTS:
    void foo();
};

#endif // STORAGEFRONTEND_H
