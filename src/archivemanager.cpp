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
    QSettings appSettings("Evopedia", "GUI");
    archivesBaseDir = appSettings.value("download_dir", "").toString();

    connect(&netManager, SIGNAL(finished(QNetworkReply*)), SLOT(handleNetworkFinished(QNetworkReply*)));
    connect(this, SIGNAL(archivesChanged(QList<Archive*>)), SLOT(updateDefaultLocalArchives(QList<Archive*>)));
    connect(this, SIGNAL(archivesExchanged(PartialArchive*,LocalArchive*)),
            SLOT(updateDefaultLocalArchivesUponExchange(PartialArchive*,LocalArchive*)));

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

            //TODO1 error handling
            //TODO1 add another archive type CorruptedArchive? Just ignore it?
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

void ArchiveManager::setArchivesBaseDir(QString dir)
{
    QSettings appSettings("Evopedia", "GUI");
    appSettings.setValue("download_dir", dir);
    appSettings.sync();
}

void ArchiveManager::updateRemoteArchives()
{
    netManager.get(QNetworkRequest(QUrl(EVOPEDIA_DUMP_SITE)));
}

void ArchiveManager::handleNetworkFinished(QNetworkReply *reply)
{
    /* TODO1 default timeout is rather long, we could use our own timer for that */
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(0, "Network Error",
                                tr("Can not access the network to find downloadable wikipedia archives because: %1")
                                .arg(reply->errorString()));
        reply->deleteLater();
        return;
    }
    //FIXME1 if the network does not work we need a timeout handler for error messages
    //FIXME1 is it possible that we get the data only in partial chunks?
    QString data = QString::fromUtf8(reply->readAll().constData());

    reply->deleteLater();

    /* remove all downloadable archives */
    QHash<ArchiveID, Archive *>::iterator i;
    for (i = archives.begin(); i != archives.end();) {
        DownloadableArchive *a = qobject_cast<DownloadableArchive *>(i.value());
        if (a) {
            i = archives.erase(i);
            /* TODO0 test */
            a->deleteLater();
        } else {
            ++ i;
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

    if (archives.contains(id)) {
        /* only add archive if it is really "more local" than the present one */

        Archive *current(archives[id]);

        int curNum = 2;
        if (qobject_cast<LocalArchive *>(current)) {
            curNum = 0;
        } else if (qobject_cast<PartialArchive *>(current)) {
            curNum = 1;
        }

        int newNum = 2;
        if (qobject_cast<LocalArchive *>(archive)) {
            newNum = 0;
        } else if (qobject_cast<PartialArchive *>(archive)) {
            newNum = 1;
        }

        if (newNum >= curNum && curNum < 2)
            return false;

        current->deleteLater();
        /* note that it is not removed from the settings file,
         * since the LocalArchive will overwrite it */
    }

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

    from->deleteLater();
}

void ArchiveManager::updateDefaultLocalArchivesUponExchange(PartialArchive *from, LocalArchive *to)
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    updateDefaultLocalArchives(getArchives().values());
}


void ArchiveManager::updateDefaultLocalArchives(const QList<Archive *> &archives)
{
    /* TODO1, default archive should be adjustable */
    QHash<QString, LocalArchive *> newDefaultLocalArchives;

    foreach (Archive *a, archives) {
        LocalArchive *la = qobject_cast<LocalArchive *>(a);
        if (!la) continue;
        QString lang = la->getLanguage();
        if (newDefaultLocalArchives.contains(lang) &&
                !(*la < *newDefaultLocalArchives[lang]))
            continue;
        newDefaultLocalArchives[lang] = la;
    }

    if (newDefaultLocalArchives != defaultLocalArchives) {
        defaultLocalArchives = newDefaultLocalArchives;
        emit defaultLocalArchivesChanged(defaultLocalArchives.values());
    }
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
