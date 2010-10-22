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
#include "downloadablearchive.h"
#include "partialarchive.h"

ArchiveManager::ArchiveManager(QObject* parent) : QObject(parent)
{
    connect(&netManager, SIGNAL(finished(QNetworkReply*)), SLOT(networkFinished(QNetworkReply*)));
    connect(this, SIGNAL(archivesChanged(QList<Archive*>)), SLOT(updateDefaultLocalArchives(QList<Archive*>)));

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (settings.contains("evopedia/data_directory")) {
        /* update 'old format' stuff */
        QString data_dir(settings.value("evopedia/data_directory").toString());
        settings.remove("evopedia/data_directory");
        settings.setValue("dump_UNKNOWN/data_directory", data_dir);
        settings.sync();
    }

    restoreLocalAndPartialArchives(settings);

    emit archivesChanged(archives.values());
}

void ArchiveManager::restoreLocalAndPartialArchives(QSettings &settings)
{
    /* TODO restart downloads */
    foreach (QString group, settings.childGroups()) {
        if (!group.startsWith("dump_"))
            continue;

        settings.beginGroup(group);

        if (settings.value("complete", true).toBool()) {
            LocalArchive *archive = LocalArchive::restoreArchive(settings, this);
            if (archive) {
                if (!addArchiveInternal(archive))
                    delete archive;
            }

            //TODO error handling
            //TODO add another archive type CorruptedArchive? Just ignore it?
            /*
            QMessageBox::critical ( NULL, "session restore: previously used evopedia archives",
                                QString("'%1' could not be opened because: '%2'. Are you still in USB-mass storage mode or have the files moved?")
                                .arg(data_dir)
                                .arg(ret));
              */
        } else {
            PartialArchive *archive = PartialArchive::restoreArchive(settings, this);
            if (archive) {
                if (!addArchiveInternal(archive))
                    delete archive;
            }
        }
        settings.endGroup();
    }
}

void ArchiveManager::updateRemoteArchives()
{
    netManager.get(QNetworkRequest(QUrl(EVOPEDIA_DUMP_SITE)));
}

void ArchiveManager::setDownloadsPaused(bool value)
{
    foreach (Archive *a, archives) {
        PartialArchive *pa = qobject_cast<PartialArchive *>(a);
        if (pa)
            pa->setExternallyPaused(value);
    }
}

void ArchiveManager::networkFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(0, "network error",
                                QString("Can not access the network to find evopedia torrents because: '%1'")
                                .arg(reply->errorString()));
        return;
    }
    //FIXME if the network does not work we need a timeout handler for error messages
    //FIXME is it possible that we get the data only in partial chunks?
    QString data = QString::fromUtf8(reply->readAll().constData());

    /* remove all downloadable archives */
    QHash<ArchiveID, Archive *>::iterator i;
    for (i = archives.begin(); i != archives.end(); i ++) {
        DownloadableArchive *a = qobject_cast<DownloadableArchive *>(i.value());
        if (a) {
            i = archives.erase(i);
            // TODO a->deleteLater();
        }
    }

    /* parse the list and add (new) downloadable archives again */

    QRegExp rx("<!-- METAINFO ([^>]*/wikipedia_([a-z_-]*)_([0-9-]*)\\.torrent) ([0-9]*) -->");
    rx.setMinimal(true);

    for (int pos = 0; (pos = rx.indexIn(data, pos)) != -1; pos += rx.matchedLength()) {
        QUrl url(rx.cap(1));
        QString language(rx.cap(2));
        QString date(rx.cap(3));
        QString size(rx.cap(4));

        if (archives.contains(ArchiveID(language, date)))
            continue;

        archives[ArchiveID(language, date)] =
                new DownloadableArchive(language, date, url, size, this);
    }

    emit archivesChanged(archives.values());
}

bool ArchiveManager::addArchiveInternal(Archive *archive)
{
    ArchiveID id(archive->getID());

    if (!archives.contains(id)) {
        archives[id] = archive;
        archive->setParent(this);
        return true;
    }

    /* only add archive if it is really "more local" than the present one */

    /* TODO this will not "refresh" information on the same
     * downloadable archive. Is that bad? */
    if (qobject_cast<DownloadableArchive *>(archive))
        return false;

    Archive *other(archives[id]);

    if (qobject_cast<LocalArchive *>(other))
        return false;

    if (qobject_cast<PartialArchive *>(archive) &&
            qobject_cast<PartialArchive *>(other))
        return false;

    if (qobject_cast<LocalArchive *>(archive) &&
            qobject_cast<LocalArchive *>(other))
        return false;

    archives[id]->deleteLater();
    /* note that it is not removed from the settings,
     * since the LocalArchive will overwrite it */

    archives[id] = archive;
    archive->setParent(this);

    return true;
}


bool ArchiveManager::addArchiveAndStoreInSettings(Archive *a)
{
    if (addArchiveInternal(a)) {
        QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
        a->saveToSettings(settings);
        return true;
    } else {
        return false;
    }
}

bool ArchiveManager::addArchive(Archive* archive)
{
    if (addArchiveAndStoreInSettings(archive)) {
        emit archivesChanged(archives.values());
        return true;
    } else {
       return false;
    }
#if 0
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
#endif
}

void ArchiveManager::exchangeArchives(DownloadableArchive *from, PartialArchive *to)
{
    if (!from || !to || from->getID() != to->getID() || !archives.contains(from->getID()))
        return;

    addArchiveAndStoreInSettings(to);

    emit archivesExchanged(from, to);

    from->deleteLater();
}

void ArchiveManager::exchangeArchives(PartialArchive *from, LocalArchive *to)
{
    if (!from || !to || from->getID() != to->getID() || !archives.contains(from->getID()))
        return;

    addArchiveAndStoreInSettings(to);

    emit archivesExchanged(from, to);
    updateDefaultLocalArchives(archives.values());

    from->deleteLater();
}


void ArchiveManager::updateDefaultLocalArchives(const QList<Archive *> &archives)
{
    /* TODO, default archive should be adjustable */
    defaultLocalArchives.empty();

    foreach (Archive *a, archives) {
        LocalArchive *la = qobject_cast<LocalArchive *>(a);
        if (!la) continue;
        QString lang = la->getLanguage();
        if (defaultLocalArchives.contains(lang) &&
                !(*la < *defaultLocalArchives[lang]))
            continue;
        defaultLocalArchives[lang] = la;
    }
    /* TODO check if there really was a change */
    emit defaultLocalArchivesChanged(defaultLocalArchives.values());
}

const QHash<QString, LocalArchive *> ArchiveManager::getDefaultLocalArchives() const
{
    return defaultLocalArchives;
}

LocalArchive *ArchiveManager::getLocalArchive(const QString language, const QString date) const
{
    if (date.isEmpty()) {
        if (defaultLocalArchives.contains(language))
            return defaultLocalArchives[language];
    } else if (archives.contains(ArchiveID(language, date))) {
        return qobject_cast<LocalArchive *>(archives[ArchiveID(language, date)]);
    }
    return 0;
}

LocalArchive *ArchiveManager::getRandomLocalArchive() const
{
    QList<LocalArchive *> archives = defaultLocalArchives.values();
    quint32 numArticles = 0;
    foreach (LocalArchive *b, archives)
        numArticles += b->getNumArticles();
    quint32 articleId = randomNumber(numArticles);
    foreach (LocalArchive *b, archives) {
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
    return defaultLocalArchives.contains(language);
}
