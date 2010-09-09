#include "archivemanager.h"

#include <QSettings>
#include <QStringList>
#include <QDir>
#include <QUrl>
#include <QMessageBox>
#include "utils.h"

ArchiveManager::ArchiveManager(QObject* parent) : QObject(parent) {
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (settings.contains("evopedia/data_directory")) {
        /* old format */
        QString data_dir(settings.value("evopedia/data_directory").toString());
        settings.remove("evopedia/data_directory");
        settings.setValue("dump_UNKNOWN/data_directory", data_dir);
        settings.sync();
        connect(&netManager, SIGNAL(finished(QNetworkReply*)), SLOT(networkFinished(QNetworkReply*)));
        updateRemoteArchives();
    }

    foreach (QString group, settings.childGroups()) {
        if (!group.startsWith("dump_"))
            continue;
        QString data_dir(settings.value(group + "/data_directory").toString());
        StorageBackend *backend = new StorageBackend(data_dir, this);
        if (!backend->isReadable()) {
            delete backend;
        } else {
            if (group.indexOf('_', 5) < 0) {
                /* old format, convert */
                settings.remove(group);
                settings.setValue(QString("dump_%1_%2/data_directory")
                                  .arg(backend->getLanguage(), backend->getDate()),
                                      backend->getDirectory());
                settings.sync();
            }
            storages[backend->getLanguage()] += backend;
        }
    }

    for (QHash<QString, QList<StorageBackend *> >::iterator
            i = storages.begin(); i != storages.end(); i ++)
        qSort(*i);
}

StorageBackend *ArchiveManager::getBackend(const QString language, const QString date) const
{
    if (!storages.contains(language))
        return 0;

    QList<StorageBackend *> backends = storages[language];
    if (backends.isEmpty())
        return 0;
    if (date.isEmpty())
        return backends[0];
    foreach (StorageBackend *b, backends) {
        if (b->getDate() == date)
            return b;
    }
    return 0;
}

const QList<StorageBackend *> ArchiveManager::getBackends() const
{
    /* TODO is this efficient enough? */
    QList<StorageBackend *>backends;
    foreach (QList<StorageBackend *>backends_it, storages)
        backends += backends_it;
    return backends;
}

StorageBackend *ArchiveManager::getRandomBackend() const
{
    quint32 numArticles = 0;
    foreach (QList<StorageBackend *>l, storages)
        foreach (StorageBackend *b, l)
            numArticles += b->getNumArticles();

    quint32 articleId = randomNumber(numArticles);
    foreach (QList<StorageBackend *>l, storages) {
        foreach (StorageBackend *b, l) {
            quint32 bArticles = b->getNumArticles();
            if (bArticles > articleId) {
                return b;
            } else {
                articleId -= bArticles;
            }
        }
    }
    return 0;
}

void ArchiveManager::addBackend(StorageBackend *backend)
{
    if (backend == 0 || !backend->isReadable())
        return;

    backend->setParent(this);

    const QString language(backend->getLanguage());
    const QString date(backend->getDate());

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(0, tr("Error"), tr("Unable to store settings."));
        delete backend;
        return;
    }
    settings.setValue(QString("dump_%1_%2/data_directory")
                      .arg(language, date),
                          backend->getDirectory());
    StorageBackend *b2 = getBackend(language, date);
    if (b2 != 0) {
        int i = storages[language].indexOf(b2);
        storages[language][i] = backend;
        delete b2;
    } else {
        storages[language] += backend;
        qSort(storages[language]);
    }
    settings.sync();

    const QList<StorageBackend *>backends = getBackends();
    emit backendsChanged(backends);
}

void ArchiveManager::removeBackend(StorageBackend *backend)
{
    if (backend == 0) return;

    const QString language(backend->getLanguage());
    const QString date(backend->getDate());

    if (getBackend(language, date) != backend) return;

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(0, tr("Error"), tr("Error storing settings."));
        return;
    }
    settings.remove(QString("dump_%1_%2").arg(language, date));
    if (storages[language].length() == 1) {
        storages.remove(language);
    } else {
        storages[language].removeOne(backend);
    }
    delete backend;
    settings.sync();

    const QList<StorageBackend *>backends = getBackends();
    emit backendsChanged(backends);
}

/*! sets the default archive to be used for the language the archive is in*/
bool ArchiveManager::setActiveBackend(int index) {
    return false;
}

/*! add a new archive:
** - a local archive    (already downloaded)
** - a remove archive   (used for selecting downloads)
** - a torrent download (this archive is persistent until done or removed)
*/
bool ArchiveManager::addArchive(QString dir, QString &ret){
        StorageBackend *backend = new StorageBackend(dir, this);
        if (!backend->isReadable()) {
            ret=backend->getErrorMessage();
            delete backend;
        } else {
            // transfers ownership
            addBackend(backend);
            return true;
        }
        return false;
}

/*! removes an archive from the manager */
void ArchiveManager::delArchive(int index){

}

/*! updates the list of remove downloads, torrents for example */
void ArchiveManager::updateRemoteArchives() {
    netManager.get(QNetworkRequest(QUrl(EVOPEDIA_URL)));
}

// add a list of possible downloads to the ArchiveManager
void ArchiveManager::networkFinished(QNetworkReply *reply)
{
    QString data = QString::fromUtf8(reply->readAll().constData());

    // remove all remote objects, then repopulate it with up2date items
    QMutableListIterator<Archive> i(archives);
    while (i.hasNext())
        if (i.next().state == Archive::InstallationCandidate)
            i.remove();

    // wikipedia_de_2010-07-27.torrent
    QRegExp rx("wikipedia_.._....-..-..\\.torrent");
    rx.setMinimal(true);

    int pos = 0;
    while((pos = rx.indexIn(data, pos)) != -1) {
        Archive a;
        QString torrent_link = data.mid(pos, rx.matchedLength());
        a.language = torrent_link.mid(10,2);
        a.date = torrent_link.mid(13,10);
        a.url = EVOPEDIA_URL + torrent_link;
        a.size = "? GB";
        a.state = Archive::InstallationCandidate;
        archives += a;

        pos += rx.matchedLength();
    }
}
