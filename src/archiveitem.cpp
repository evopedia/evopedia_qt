#include "archiveitem.h"
#include "storagebackend.h"

#include <QMenu>
#include <QSettings>
#include <QDir>

/*! this code is needed to prevent a crash as the m_storageBackend usage is bypassing the MVC concepts */
ArchiveItem::~ArchiveItem(){
    m_enabled=false;
    //emit updateBackends();
}

/*! adding a local archive */
ArchiveItem::ArchiveItem(QString dir) : QStandardItem() {
    m_dir = dir;
    m_storageBackend=NULL;
    m_itemState=ItemState::Local;
    QString ret;
    if (validate(ret)) {
        m_size = "todo2";
    }
    store();
}

/*! adding a remote or local torrent archive */
ArchiveItem::ArchiveItem(QString language, QString date, QString dir, QString torrent, QUrl url) : QStandardItem() {
    m_language = language;
    m_date = date;
    m_url = url;
    m_enabled=false;
    m_size="todo1";
    m_storageBackend=NULL;
    // if dir is given and a valid QDIR we assume it's a LocalTorrent and we search for a torrent file in 'dir' and
    // we try to resume the download... and stuff like that
    m_dir = dir;
    m_itemState=ItemState::RemoteTorrent;
}

/*! update() might not work when being called from the ctor as the other items are most likely not added to the model yet
**  otherwise calling update() will update the other child items in the other columns next to this item */
void ArchiveItem::update() {
    if (parent()) {

      QStandardItem* i;
      i = parent()->child(row(),1);
      if (i)
          i->setText("size");
      i = parent()->child(row(),2);
      if (i)
          i->setText(m_size);
      i = parent()->child(row(),3);
      if (i)
          i->setText(m_state);
    }
}

bool ArchiveItem::validate(QString& ret) {
    if (m_storageBackend != NULL) {
        delete m_storageBackend;
        m_storageBackend=NULL;
    }
    StorageBackend *backend = new StorageBackend(m_dir);
    ret = backend->getErrorMessage();
    if (m_itemState==ItemState::RemoteTorrent)
        m_state = "remote torrent, download it?";
    else
        m_state = ret;
    if (!backend->isReadable()) {
         delete backend;
         m_storageBackend=NULL;
     } else {
         m_storageBackend = backend;
         m_language = m_storageBackend->getLanguage();
         m_date = m_storageBackend->getDate();
         update();
         return true;
     }
     update();
     return false;
}

void ArchiveItem::changeBackend(int type) {
    switch(type) {
    case 0: // localArchive
        // new blah
        break;
    case 1: // torrentArchive (either complete or incomplete)
        break;
    }
}

void ArchiveItem::extend() {
    QStandardItem *item1_1Col1 = new QStandardItem(QString("item1_1Col2")); // Spalte 1 vom ChildChild
    QStandardItem *item1_1Col2 = new QStandardItem(QString("item1_1Col2")); // Spalte 2 vom ChildChild
    QStandardItem *item1_1Col3 = new QStandardItem(QString("item1_1Col2")); // Spalte 2 vom ChildChild

    // Spalten vorbereiten
    QList<QStandardItem*> itemList;
    itemList.append(item1_1Col1);
    itemList.append(item1_1Col2);
    itemList.append(item1_1Col3);
    appendRow(itemList); // Child an Child hngen
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

/*! used to convert QStandardItem(s) into ArchiveItem(s) */
int ArchiveItem::type() const {
    return QStandardItem::UserType + 1;
}

StorageBackend *ArchiveItem::storageBackend() {
    return m_storageBackend;
}

void ArchiveItem::setState(QString state) {
    m_state = state;
    update();
}

QMenu* ArchiveItem::createContextMenu() {
    QMenu* m = new QMenu();
    m->addAction("remote entry");
    m->addAction("remote entry and delete files");
    m->addAction("pause torrent download");
    m->addAction("resume torrent download");
    m->addAction("cancel torrent download & remove files");
    return m;
}


void ArchiveItem::store() {
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        //FIXME
        //QMessageBox::critical(0, tr("Error"), tr("Unable to store settings."));
        return;
    }
    settings.setValue(QString("dump_%1_%2/data_directory")
                      .arg(language(), date()), dir());
    settings.sync();
}
