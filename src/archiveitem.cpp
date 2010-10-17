#include <QMenu>
#include <QSettings>
#include <QMessageBox>
#include <QObject>
#include <QDir>
#include <QUrl>
#include <QDebug>

#include "archiveitem.h"
#include "localarchive.h"
#include "storagefrontend.h"

/*! this code is needed to prevent a crash as the m_storageBackend usage is bypassing the MVC concepts */
ArchiveListItem::~ArchiveListItem() {
    if (m_storagefrontend)
         delete m_storagefrontend;
    if (m_torrentfrontend)
         delete m_torrentfrontend;
}

/*! adding a local archive */
ArchiveListItem::ArchiveListItem(QString archiveDir) : QStandardItem() {
    // initialize local variables first (above), before creating the frontends
    m_storagefrontend = new StorageFrontend(this, archiveDir);
    m_torrentfrontend=NULL;
    m_itemState=ItemState::Local;
}

/*! adding a remote or local torrent archive */
ArchiveListItem::ArchiveListItem(QString language, QString date, QString size, QString workingDir, QString archiveDir, QString torrent, QUrl url) : QStandardItem() {
    m_torrentfrontend = new TorrentFrontend(this, language, date, size, workingDir, archiveDir, torrent, url);
    m_storagefrontend = new StorageFrontend(this);
    m_itemState=ItemState::DownloadableArchive;
}

/*! update() might not work when being called from the ctor as the other items are most likely not added to the model yet
**  otherwise calling update() will update the other child items in the other columns next to this item */
void ArchiveListItem::update() {
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
          i->setText(stateString());
    }
}

QString ArchiveListItem::language() {
    if (itemState() == ItemState::DownloadableArchive || itemState() == ItemState::LocalTorrent)
        return m_torrentfrontend->language();
    if (itemState() == ItemState::Local && m_storagefrontend)
        return m_storagefrontend->language();
    return QString();
}

QString ArchiveListItem::date() {
    if (itemState() == ItemState::DownloadableArchive || itemState() == ItemState::LocalTorrent)
        return m_torrentfrontend->date();
    if (itemState() == ItemState::Local && m_storagefrontend)
        return m_storagefrontend->date();
    return QString();
}

QString ArchiveListItem::dir() {
    if (itemState() == ItemState::DownloadableArchive || itemState() == ItemState::LocalTorrent)
        return m_torrentfrontend->archiveDir();
    if (itemState() == ItemState::Local && m_storagefrontend)
        return m_storagefrontend->archiveDir();
    return QString();
}

QUrl ArchiveListItem::url() {
    if (itemState() == ItemState::DownloadableArchive || itemState() == ItemState::LocalTorrent)
        return m_torrentfrontend->url();
    return QUrl();
}

QString ArchiveListItem::size() {
    if (itemState() == ItemState::DownloadableArchive || itemState() == ItemState::LocalTorrent)
        return m_torrentfrontend->size();
    if (itemState() == ItemState::Local && m_storagefrontend)
        return m_storagefrontend->size();
    return QString();
}


/*! text for the state-column */
QString ArchiveListItem::stateString() {
    if (itemState() == ItemState::DownloadableArchive || itemState() == ItemState::LocalTorrent)
        return m_torrentfrontend->stateString();
    if (itemState() == ItemState::Local && m_storagefrontend)
        return m_storagefrontend->stateString();
    return QString();
}

LocalArchive *ArchiveListItem::storageBackend() {
    if (m_storagefrontend)
        return m_storagefrontend->storageBackend();
    return NULL;
}

/*! can this storagebackend be used? */
bool ArchiveListItem::activated() {
    if (!m_storagefrontend)
        return false;
    return m_storagefrontend->m_activated;
}

bool ArchiveListItem::validate(QString& ret) {
    if (!m_storagefrontend)
        return false;
    return m_storagefrontend->validate(ret);
}

/*! used to change the backend used for the item, local vs torrent */
int ArchiveListItem::itemState() {
    return m_itemState;
}

void ArchiveListItem::setItemState(int s) {
    if (itemState() == ItemState::DownloadableArchive && s == ItemState::DownloadingTorrent) {
//        qDebug() << __PRETTY_FUNCTION__ << "changing state to ItemState::DownloadingTorrent";
        m_itemState = s;
    }
    if (itemState() == ItemState::DownloadingTorrent && s == ItemState::LocalTorrent) {
//        qDebug() << __PRETTY_FUNCTION__ << "changing state to ItemState::LocalTorrent";
        m_storagefrontend->setArchiveDirectory(m_torrentfrontend->archiveDir());
    }
    //FIXME if a torrent download is started successfully:
    //      ItemState::RemoteTorrent will change to ItemState::LocalTorrent

    //FIXME if a torrent download is complete we need to enable the archive
    //      this will change the state from ItemState::LocalTorrent to ItemState::Local
    //      prior to that we need to create a StorageFrontend object
}

/*! used to convert QStandardItem(s) into ArchiveItem(s), QStandardItem related */
int ArchiveListItem::type() const {
    return QStandardItem::UserType + 1;
}

/*! removes the entry from the (QStandardItemModel) QTreeView */
void ArchiveListItem::removeEntry() {
    model()->removeRow(0, parent()->index());
}

QMenu* ArchiveListItem::createContextMenu() {
    QMenu* main = new QMenu();
    QMenu* m1;
    if (m_storagefrontend) {
        m1 = m_storagefrontend->createContextMenu();
        m1->setTitle("Local");
        main->addMenu(m1);
    }
    if (m_torrentfrontend) {
        QMenu* m2 = m_torrentfrontend->createContextMenu();
        m2->setTitle("Torrent");
        main->addMenu(m2);
    }
    return main;
}
