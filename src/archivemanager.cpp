#include "archivemanager.h"
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QUrl>
#include <QStandardItemModel>
#include <QDebug>
#include <QModelIndex>

#include "utils.h"

ArchiveManager::ArchiveManager(QObject* parent) : QObject(parent) {
    // QTreeView properties of dumpSettings
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(4);
    m_model->setHeaderData(0, Qt::Horizontal, QString("lang/date"));
    m_model->setHeaderData(1, Qt::Horizontal, QString(""));
    m_model->setHeaderData(2, Qt::Horizontal, QString(""));
    m_model->setHeaderData(3, Qt::Horizontal, QString("status"));

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (settings.contains("evopedia/data_directory")) {
        /* update 'old format' stuff */
        QString data_dir(settings.value("evopedia/data_directory").toString());
        settings.remove("evopedia/data_directory");
        settings.setValue("dump_UNKNOWN/data_directory", data_dir);
        settings.sync();
    }
    connect(&netManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(networkFinished(QNetworkReply*)));

    // restore evopedia archives (local evopedia installations)
    foreach (QString group, settings.childGroups()) {
        if (!group.startsWith("dump_"))
            continue;
        QString data_dir(settings.value(group + "/data_directory").toString());
        QString ret;
        ArchiveItem* archiveItem = addArchive(data_dir, ret);
        if (archiveItem==NULL) {
            //TODO find a nice way how to handle this
            QMessageBox::critical ( NULL, "session restore: previously used evopedia archives",
                                QString("'%1' could not be opened because: '%2'. Are you still in USB-mass storage mode or have the files moved?")
                                .arg(data_dir)
                                .arg(ret));
        }
        if (archiveItem) {
            //FIXME please check this code, is that correct? (js)
            if (group.indexOf('_', 5) < 0) {
                 // old format, convert
                 settings.remove(group);
                 settings.setValue(QString("dump_%1_%2/data_directory")
                    .arg(archiveItem->language(), archiveItem->date()),
                 archiveItem->dir());
                 settings.sync();
             }
         }
    // FIXME: restore torrent archives
    }
    emit updateBackends();
}

/*! updates the list of remote downloads, torrents for example */
void ArchiveManager::updateRemoteArchives() {
    netManager.get(QNetworkRequest(QUrl(EVOPEDIA_URL)));
}

/*! updates the list of remote downloads, torrents for example */
void ArchiveManager::networkFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical ( NULL, "network error",
                                QString("Can not access the network to find evopedia torrents because: '%1'")
                                .arg(reply->errorString()));
        return;
    }
    //FIXME if the network does not work we need a timeout handler for error messages
    QString data = QString::fromUtf8(reply->readAll().constData());

    // wikipedia_de_2010-07-27.torrent
    QRegExp rx("wikipedia_.._....-..-..\\.torrent");
    rx.setMinimal(true);

    // let's remove all old RemoteTorrent(s) first
    QVector<ArchiveItem*> c;
    for(int i=0; i < m_model->rowCount(); ++i) {
        QStandardItem* langItem = m_model->item(i,0);
        for(int y=0; y < langItem->rowCount();) {
            QStandardItem* sItem = langItem->child(y,0);
            // 1. type check
            if (sItem->type() == QStandardItem::UserType + 1) {
                // 2. type cast
                ArchiveItem* item = static_cast<ArchiveItem*>(sItem);
                // 3. delete all ItemState::RemoteTorrent objects
                if (item->itemState() == ItemState::RemoteTorrent)
                    m_model->removeRow(item->row(), item->parent()->index());
                ++y;
            }
        }
    }

    int pos = 0;
    // now we parse the html page to generate new RemoteTorrent(s)
    while((pos = rx.indexIn(data, pos)) != -1) {
        // FIXME use ret and display the message
        // FIXME the use of EVOPEDIA_URL to construct the torrent link is problematic
        //       rewrite this to use a regexp to find the whole url as:
        //       http://evopedia.info/dumps/wikipedia_de_2010-07-27.torrent
        QString ret; // return argument
        QString torrent_link = data.mid(pos, rx.matchedLength());
        QUrl url(EVOPEDIA_URL + torrent_link);
        QString language = torrent_link.mid(10,2);
        QString date = torrent_link.mid(13,10);
        QString torrent = "";
        QString dir("/tmp");
        /*
        wget "http://dumpathome.evopedia.info/dumps/finished" -O dumps;
        cat dumps | grep 'wikipedia_.._....-..-...torrent' | grep META | grep -o 'http://.*torrent [0-9]*'

        http://evopedia.info/dumps/wikipedia_de_2010-07-27.torrent; 2842735141
        http://evopedia.info/dumps/wikipedia_el_2010-08-15.torrent; 156453782
        http://evopedia.info/dumps/wikipedia_en_2010-06-22.torrent; 11297506467
        http://evopedia.info/dumps/wikipedia_fr_2010-08-02.torrent; 2953095765
        http://evopedia.info/dumps/wikipedia_it_2010-09-02.torrent; 2363497456
        http://evopedia.info/dumps/wikipedia_sl_2010-08-12.torrent; 216360675
        */

        //FIXME only add a 'remote archive' if it is not there already
        //addArchive(language, date, dir, torrent, url, ret);
        pos += rx.matchedLength();
    }
    QString ret; // return argument
    addArchive("de", "2010-07-27", "", "wikipedia_de_2010-07-27.torrent", QUrl("http://evopedia.info/dumps/wikipedia_de_2010-07-27.torrent"), ret);
    addArchive("sl", "2010-08-12", "", "wikipedia_sl_2010-08-12.torrent", QUrl("http://evopedia.info/dumps/wikipedia_sl_2010-08-12.torrent"), ret);
    addArchive("small", "2010-08-14", "", "wikipedia_small_2010-08-14.torrent", QUrl("http://evopedia.info/dumps/wikipedia_small_2010-08-14.torrent"), ret);
}

/*
 - lang
 - date
 - dir (where to store, where to find)
 - url (example: http://evopedia.info/dumps/)
 - torrent (example: wikipedia_de_2010-07-27.torrent)
 - status indicator: active download or download candidate
 - return string (storageBackend)
*/

/*! used for local archives, manual downloads*/
ArchiveItem* ArchiveManager::addArchive(QString dir, QString& ret) {
    ArchiveItem* item = new ArchiveItem(dir);
    if (!item->validate(ret)) {
        delete item;
        return NULL;
    }
    return addArchive(item);
}

/*! used for torrent based local archives which either have the *.torrent still around or not */
ArchiveItem* ArchiveManager::addArchive(QString language, QString date, QString dir, QString torrent, QUrl url, QString& ret) {
    ArchiveItem* item = new ArchiveItem(language, date, dir, torrent, url);
    item->setData(true, Qt::UserRole + 1);
    item->validate(ret);
    return addArchive(item);
}

ArchiveItem* ArchiveManager::addArchive(ArchiveItem* item) {
    connect(item->m_storagefrontend, SIGNAL(updateBackends()), SLOT(updateBackends()));

    // 1. find the language group
    QStandardItem* langItem = NULL;
    for(int i=0; i < m_model->rowCount(); ++i) {
        if (m_model->item(i,0)->text() == item->language()) {
            langItem = m_model->item(i,0);
            break;
        }
    }
    // 2. if no langItem found, we create it
    if (langItem == NULL) {
        langItem = new QStandardItem(item->language());
        langItem->setEditable(false);
        QStandardItem *item1_1Col2 = new QStandardItem();  // column 2 (starting by 1)
        QStandardItem *item1_1Col3 = new QStandardItem();  // column 3 (starting by 1)
        QStandardItem *item1_1Col4 = new QStandardItem();  // column 4 (starting by 1)
        item1_1Col2->setEditable(false);
        item1_1Col3->setEditable(false);
        item1_1Col4->setEditable(false);
        QList<QStandardItem*> itemList;
        itemList.append(langItem);
        itemList.append(item1_1Col2);
        itemList.append(item1_1Col3);
        itemList.append(item1_1Col4);
        m_model->appendRow(itemList);
    }

    // 3. append it as child with the right type
    QStandardItem *item1_1Col2 = new QStandardItem();  // column 2 (starting by 1)
    QStandardItem *item1_1Col3 = new QStandardItem();  // column 3
    QStandardItem *item1_1Col4 = new QStandardItem();  // column 4 (starting by 1)
    item->setEditable(false);
    item->setText(item->date());
    item1_1Col2->setEditable(false);
    item1_1Col3->setEditable(false);
    item1_1Col4->setEditable(false);
    // Spalten vorbereiten
    QList<QStandardItem*> itemList;
    itemList.append(item);
    itemList.append(item1_1Col2);
    itemList.append(item1_1Col3);
    itemList.append(item1_1Col4);
    langItem->appendRow(itemList); // Child an Child haengen
    item->update(); // mendatory function to update the all items in one row using the model

    emit updateBackends();
    return item;
}

QStandardItemModel* ArchiveManager::model() {
    return m_model;
}

/*! whenever a valid archive is added or setDefaultArchive(..) is used this function must be called
**  also before an archive is removed while that archive has enabled=false set */
void ArchiveManager::updateBackends()
{
    //FIXME rowsAboutToBeRemoved singal should be linked here as well
    const QList<StorageBackend *>backends = getBackends();
    emit backendsChanged(backends);
}

/*! retunrs a list of backends by parsing the archives structure:
** - validate() == true
** - activated() == true
*/
const QList<StorageBackend *> ArchiveManager::getBackends() const
{
    QList<StorageBackend *>backends;
    for(int i=0; i < m_model->rowCount(); ++i) {
        QStandardItem* langItem = m_model->item(i,0);
        for(int y=0; y < langItem->rowCount(); ++y) {
            QStandardItem* sItem = langItem->child(y,0);
            // 1. type check
            if (sItem->type() == QStandardItem::UserType + 1) {
                // 2. type cast
                ArchiveItem* item = static_cast<ArchiveItem*>(sItem);
                // 3. add the backend
                QString r;
                if (item->validate(r) && item->activated()) {
                    if (item->storageBackend())
                        backends += item->storageBackend();
                }
            }
        }
    }
    return backends;
}

StorageBackend *ArchiveManager::getBackend(const QString language, const QString date) const
{
    //FIXME the data object seems to be unused, why?!
    //qDebug() << language << date;
    QList<StorageBackend *> backends = getBackends();
    if (backends.size()) {
        foreach(StorageBackend* b, backends) {
            if (b->getLanguage() == language /*&& b->getDate() == date*/)
                return b;
        }
    }
    return 0;
}

StorageBackend *ArchiveManager::getRandomBackend() const
{
    QList<StorageBackend *> backends =  getBackends();
    quint32 numArticles = 0;
    foreach (StorageBackend *b, backends)
        numArticles += b->getNumArticles();
    quint32 articleId = randomNumber(numArticles);
    foreach (StorageBackend *b, backends) {
        quint32 bArticles = b->getNumArticles();
        if (bArticles > articleId) {
            return b;
        } else {
            articleId -= bArticles;
        }
    }

    return 0;
}

bool ArchiveManager::hasLanguage(const QString language) const {
    QList<StorageBackend *> backends =  getBackends();
    if (backends.size()) {
        foreach(StorageBackend* b, backends) {
            if (b->getLanguage() == language)
                return true;
        }
    }
    return false;
}
