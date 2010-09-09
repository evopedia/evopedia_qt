/*
===============================================================
 how to find torrents
===============================================================
this regexp filter is needed to get all torrent links
  wikipedia_.._....-..-..\.torrent

source: dumpathome.evopedia.info/dumps/finished

===============================================================
 how to download
===============================================================
the evopedia::ArchiveManager tracks the installed evopedias based on the directory/name structure policy

wikipedia_de_2010-07-27.torrent <--> wikipedia_de_2010-07-27/
- if a new download (torrent) is started:
    then we check the evopedia::torrent_downloader history if
    a $whatever/wikipedia_de_2010-07-27_incomplete/ exists
    if so, we resume
 -> downloads have save/resume features, so the evopedia app can be
    started again later and the previous progress is not lost.
- if a download is complete, the directory is renamed:
  whatever/wikipedia_de_2010-07-27_incomplete/ <--> whatever/wikipedia_de_2010-07-27
  this step is important for seeding and not to download twice
  -> directories with '_incomplete' in the name can't be used

- wikipedia_de_2010-07-27.torrent - torrent files are stored in
  ~/.local/share/evopedia/torrents
  -> unused *.torrent files are removed if the respective archive
     is not found/existent anymore

===============================================================
 how to deploy
===============================================================
all files in a torrent, example:
  http://evopedia.info/dumps/wikipedia_en_2010-06-22.torrent
must be in a respective directory, example:
  wikipedia_en_2010-06-22/

i will build an algorithm to enforce that policy

General:
 - if a download was started, resume it when this application is started
 - the netManager should only be started when the gui is actually shown (show()), not on program start
 -> that has something to do with the 'unintuitive' design of the 'refresh' button, which
    took me quite some time to figure out that it 'actually' existet ;P
 - the size/contents of the torrents can be queried when contacting the tracker
*/

#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include <QList>
#include <QHash>
#include <QAbstractItemModel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>

#include "storagebackend.h"
#include "archive.h"

#define EVOPEDIA_URL "http://dumpathome.evopedia.info/dumps/finished"

/*! manages different wikipedia dumps (called Archive)
** - this class should be instantiated only once
** - manages torrent downloads of archives
** - sets the default archives for each language
**   one may have installed several 'de' archives, but only
**   one can be used at a time.
** - manages archives, adds(downloads or removes them)
** - implements a QAbstractItemModel to visualize what is going on
*/
class ArchiveManager : public QObject {
Q_OBJECT
public:
    explicit ArchiveManager(QObject* parent);

    // functions to handle archives (invalid backends)
    bool setActiveBackend(int index);
    bool addArchive(QString dir, QString& ret);
    void delArchive(int index);
    void updateRemoteArchives();

    // functions to handle backends (valid archives)
    StorageBackend *getBackend(const QString language, const QString date=QString()) const;
    StorageBackend *getRandomBackend() const;
    const QList<StorageBackend *> getBackends() const;
    bool hasLanguage(const QString language) const { return storages.contains(language); }

signals:
    void backendsChanged(const QList<StorageBackend *> backends);

private slots:
        void networkFinished(QNetworkReply *reply);
private:
    QHash<QString, QList<StorageBackend *> > storages;
    QNetworkAccessManager netManager;
    QList<Archive> archives;
    void addBackend(StorageBackend *backend);
    void removeBackend(StorageBackend *backend);
};

#endif
