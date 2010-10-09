#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QDir>
#include <QMessageBox>

#include "storagefrontend.h"
#include "archiveitem.h"

StorageFrontend::StorageFrontend(ArchiveItem* item) {
    m_archiveitem = item;
    m_storageBackend=NULL;
    QString ret;
    if (validate(ret)) {
        saveSettings(); // store this archive for session resume only if it is valid once
        m_activated=true;
    }
}

StorageFrontend::~StorageFrontend() {
}

QMenu* StorageFrontend::createContextMenu() {
    QMenu* m = new QMenu();

    QAction* removeEntryAct = new QAction(QIcon(), "remove entry", this);
    connect(removeEntryAct, SIGNAL(triggered()), this, SLOT(removeEntry()));
    m->addAction(removeEntryAct);

    /*
    m->addAction("remote entry and delete files");
    */
    return m;
}

/*! removes the dump from the QTreeView list and removes it from the session handler, will not delete files */
void StorageFrontend::removeEntry() {
    m_activated=false;
    emit updateBackends();
    unsaveSettings();
    m_archiveitem->removeEntry();
}

bool StorageFrontend::validate(QString& ret) {
    //FIXME maybe this code can implement better (js)
    if (m_storageBackend) {
        delete m_storageBackend;
        m_storageBackend=NULL;
    }
    StorageBackend *backend = new StorageBackend(m_archiveitem->dir());
    ret = backend->getErrorMessage();
    m_archiveitem->setStateString(ret);
    if (!backend->isReadable()) {
         delete backend;
         m_storageBackend=NULL;
     } else {
         m_storageBackend = backend;
         m_archiveitem->m_language = m_storageBackend->getLanguage();
         m_archiveitem->m_date = m_storageBackend->getDate();
         m_archiveitem->update();
         return true;
     }
     m_archiveitem->update();
     return false;
}

/*! uses qsettings to store this archive for program resume after exit */
void StorageFrontend::saveSettings() {
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Unable to store settings."));
        return;
    }
    settings.setValue(QString("dump_%1_%2/data_directory")
                      .arg(m_archiveitem->language(), m_archiveitem->date()), m_archiveitem->dir());
    settings.sync();
}

/*! uses qsettings to remove previous stores */
void StorageFrontend::unsaveSettings() {
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Unable to store settings."));
        return;
    }
    settings.remove(QString("dump_%1_%2/data_directory")
                      .arg(m_archiveitem->language(), m_archiveitem->date()));
    settings.sync();
}

StorageBackend *StorageFrontend::storageBackend() {
    return m_storageBackend;
}
