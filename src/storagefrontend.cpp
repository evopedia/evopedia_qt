#include "storagefrontend.h"
#include <QMenu>
#include <QAction>
#include <QDebug>

StorageFrontend::StorageFrontend() {
}

StorageFrontend::~StorageFrontend() {
    //m_activated=false;
    emit updateBackends();
}

QMenu* StorageFrontend::createContextMenu() {
    QMenu* m = new QMenu();

    QAction* openAct = new QAction(QIcon(), "remove entry", this);
    connect(openAct, SIGNAL(triggered()), this, SLOT(foo()));

    m->addAction(openAct);
    /*
    m->addAction("remote entry and delete files");
    m->addAction("pause torrent download");
    m->addAction("resume torrent download");
    m->addAction("cancel torrent download & remove files");
    */
    return m;
}

void StorageFrontend::foo() {
    qDebug() << __PRETTY_FUNCTION__;
}
