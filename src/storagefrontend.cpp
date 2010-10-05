#include "storagefrontend.h"
#include <QMenu>
#include <QAction>
#include <QDebug>

#include <archiveitem.h>

StorageFrontend::StorageFrontend(ArchiveItem* item) {
    m_archiveitem = item;
}

StorageFrontend::~StorageFrontend() {
}

QMenu* StorageFrontend::createContextMenu() {
    QMenu* m = new QMenu();

    QAction* openAct = new QAction(QIcon(), "remove entry", this);
    connect(openAct, SIGNAL(triggered()), this, SLOT(removeEntry()));

    m->addAction(openAct);
    /*
    m->addAction("remote entry and delete files");
    m->addAction("pause torrent download");
    m->addAction("resume torrent download");
    m->addAction("cancel torrent download & remove files");
    */
    return m;
}

void StorageFrontend::removeEntry() {
    m_archiveitem->m_activated=false;
    emit updateBackends();
    m_archiveitem->unstore();
    m_archiveitem->removeEntry();
}
