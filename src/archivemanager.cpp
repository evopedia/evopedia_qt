#include "archivemanager.h"
#include <QSettings>
#include <QStringList>
#include <QDir>
#include <QUrl>
#include <QMessageBox>
#include <QStandardItemModel>

#include "utils.h"

ArchiveManager::ArchiveManager(QObject* parent) : QObject(parent) {
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(3);

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (settings.contains("evopedia/data_directory")) {
        /* update 'old format' stuff */
        QString data_dir(settings.value("evopedia/data_directory").toString());
        settings.remove("evopedia/data_directory");
        settings.setValue("dump_UNKNOWN/data_directory", data_dir);
        settings.sync();
    }
    connect(&netManager, SIGNAL(finished(QNetworkReply*)), SLOT(networkFinished(QNetworkReply*)));
    QString d;
    addArchive("/home/", d);

    // restore evopedia archives (local evopedia installations)
    foreach (QString group, settings.childGroups()) {
        if (!group.startsWith("dump_"))
            continue;
        QString data_dir(settings.value(group + "/data_directory").toString());
        QString ret;
        //FIXME maybe we are interested in a error message? (js)
        if (addArchive(data_dir, ret)) {
            //FIXME please check this code, is that correct? (js)
            /*
            if (group.indexOf('_', 5) < 0) {
                 // old format, convert
                 settings.remove(group);
                 settings.setValue(QString("dump_%1_%2/data_directory")
                    .arg(backend->getLanguage(), backend->getDate()),
                 backend->getDirectory());
                 settings.sync();
             }
             */
         }
    // FIXME: restore torrent archives
    }
    emit updateBackends();
}

/*! updates the list of remove downloads, torrents for example */
void ArchiveManager::updateRemoteArchives() {
    netManager.get(QNetworkRequest(QUrl(EVOPEDIA_URL)));
}

// add a list of possible downloads to the ArchiveManager
void ArchiveManager::networkFinished(QNetworkReply *reply)
{
    //FIXME if the network does not work we need a timeout handler for error messages
    QString data = QString::fromUtf8(reply->readAll().constData());

    // remove all remote objects, then repopulate it with up2date items
    // IMPLEMENT THIS
    // use removeArchive(identifier);

    // wikipedia_de_2010-07-27.torrent
    QRegExp rx("wikipedia_.._....-..-..\\.torrent");
    rx.setMinimal(true);

    int pos = 0;
    while((pos = rx.indexIn(data, pos)) != -1) {
        /*
        Archive a;
        QString torrent_link = data.mid(pos, rx.matchedLength());
        a.language = torrent_link.mid(10,2);
        a.date = torrent_link.mid(13,10);
        a.url = EVOPEDIA_URL + torrent_link;
        a.size = "? GB";
        a.state = Archive::InstallationCandidate;
        addArchive(a);
        */
        pos += rx.matchedLength();
    }
}

//FIXME implement this
bool ArchiveManager::addArchive(QString dir, QString& ret) {
    ArchiveItem* item = new ArchiveItem;
    item->setText("testitem");
    m_model->setItem(0, 0, item);
}

/*! used to store the current list of Archives. so next program start one can resume using these*/
void ArchiveManager::store() {
  //FIXME implement this
}

QStandardItemModel* ArchiveManager::model() {
    return m_model;
}

/*! whenever a valid archive is added or setDefaultArchive(..) is used this function must be called
**  also befor an archive is removed while that archive has enabled=false set */
void ArchiveManager::updateBackends()
{
    //FIXME rowsAboutToBeRemoved singal should be linked here as well
    const QList<StorageBackend *>backends = getBackends();
    emit backendsChanged(backends);
}

const QList<StorageBackend *> ArchiveManager::getBackends() const
{
    //FIXME!!!
    QList<StorageBackend *>backends;
    //QHash<QString, QList<StorageBackend *> > storages;
    /*
    foreach (QList<StorageBackend *>backends_it, storages)
        backends += backends_it;
    return backends;
    */
    return backends;
}

StorageBackend *ArchiveManager::getBackend(const QString language, const QString date) const
{
    //FIXME!!!
    /*
    QHash<QString, QList<StorageBackend *> > storages = getBackends();
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
    */
    return 0;
}

StorageBackend *ArchiveManager::getRandomBackend() const
{
    //FIXME!!!
    /*
    QHash<QString, QList<StorageBackend *> > storages = getBackends();
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
    */
    return 0;
}

