#include <QMenu>
#include <QSettings>
#include <QMessageBox>
#include <QObject>
#include <QDir>
#include <QUrl>
#include <QDebug>

#include "archiveitem.h"
#include "storagebackend.h"
#include "storagefrontend.h"

/*! this code is needed to prevent a crash as the m_storageBackend usage is bypassing the MVC concepts */
ArchiveItem::~ArchiveItem() {
    if (m_storagefrontend)
         delete m_storagefrontend;
    if (m_torrentfrontend)
         delete m_torrentfrontend;
}

/*! adding a local archive */
ArchiveItem::ArchiveItem(QString dir) : QStandardItem() {
    m_dir = dir;
    // initialize local variables first (above), before creating the frontends
    m_storagefrontend = new StorageFrontend(this);
    m_torrentfrontend=NULL;
    m_itemState=ItemState::Local;
}

/*! adding a remote or local torrent archive */
ArchiveItem::ArchiveItem(QString language, QString date, QString workingDir, QString torrent, QUrl url) : QStandardItem() {
    QString archiveDir;
    m_language = language;
    m_date = date;
    m_torrent=torrent;
    m_url = url;
    m_size="todo1";
    m_dir = workingDir;
    // initialize local variables first (above), before creating the frontends
    m_storagefrontend = new StorageFrontend(this);
    m_torrentfrontend = new TorrentFrontend(this);

    // if dir is given and a valid QDIR we assume it's a LocalTorrent and we search for a torrent file in 'dir' and
    // we try to resume the download... and stuff like that
    // -> m_itemState=ItemState::LocalTorrent;
    // else
    m_itemState=ItemState::RemoteTorrent;
}

/*! update() might not work when being called from the ctor as the other items are most likely not added to the model yet
**  otherwise calling update() will update the other child items in the other columns next to this item */
void ArchiveItem::update() {
    if (parent()) {
      QStandardItem* i;
      i = parent()->child(row(),1);
      if (i)
          i->setText("");
      i = parent()->child(row(),2);
      if (i)
          i->setText("m_size");
      i = parent()->child(row(),3);
      if (i)
          i->setText(m_state);
    }
}

QString ArchiveItem::language() {
    return m_language;
}

QString ArchiveItem::date() {
    return m_date;
}

QString ArchiveItem::dir() {
    return m_dir;
}

QUrl ArchiveItem::url() {
    return m_url;
}

QString ArchiveItem::size() {
    return m_size;
}

/*! text for the state-column */
QString ArchiveItem::state() {
    return m_state;
}

/*! used to change the backend used for the item, local vs torrent */
int ArchiveItem::itemState() {
    return m_itemState;
}

/*! used to convert QStandardItem(s) into ArchiveItem(s), QStandardItem related */
int ArchiveItem::type() const {
    return QStandardItem::UserType + 1;
}

StorageBackend *ArchiveItem::storageBackend() {
    return m_storagefrontend->storageBackend();
}

void ArchiveItem::setStateString(QString state) {
    m_state = state;
    update();
}

/*! can this storagebackend be used? */
bool ArchiveItem::activated() {
    return m_storagefrontend->m_activated;
}

bool ArchiveItem::validate(QString& ret) {
    return m_storagefrontend->validate(ret);
}

/*! removes the entry from the (QStandardItemModel) QTreeView */
void ArchiveItem::removeEntry() {
    model()->removeRow(0, parent()->index());
}

QMenu* ArchiveItem::createContextMenu() {
    QMenu* main = new QMenu();
    QMenu* m1 = m_storagefrontend->createContextMenu();
    m1->setTitle("Local");
    main->addMenu(m1);
    if (m_torrentfrontend) {
        QMenu* m2 = m_torrentfrontend->createContextMenu();
        m2->setTitle("Torrent");
        main->addMenu(m2);
    }
    return main;
}
