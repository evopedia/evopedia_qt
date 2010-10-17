/*
=============================================================
IS THIS LIST STILL CURRENT?
===============================================================
  todo
===============================================================
 - make removal of local evopedias possible (really?)

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
I) all files in a torrent, example:
  http://evopedia.info/dumps/wikipedia_en_2010-06-22.torrent
must be in a respective directory, example:
  wikipedia_en_2010-06-22/

II) all filename <-> in torrent filenames must match, that is:
   - language
   - date

an algorithm will be used to enforce the policies I and II

General:
 - if a download was started, resume it when this application is started
 - the netManager:
  - should only be started when the gui is actually shown (show())
  - can be 'refresh'ed manually using a button
 - the size/contents of the torrents can be queried when contacting the tracker

===============================================================
 new design:
===============================================================
 - an archive can be used as backend if it validates correctly
 - everything is an archive:
  - local evopedias (either valid or invalid)
  - a remote object (installation candidate)
  - torrentArchive or downloads (they are stored on program exit and restored on program start)
   - a active download (torrent)
   - a paused download (torrent)
   - a finished download (torrent) can potentially seed
 - if an archive quallifies as a valid backend, it's status changes 'by user authority' into a backend
  - downloads which have finished
  - achrives which have been added manually
  - if the newly added archive is the only archive represnting a language (as 'de') it is selected as default
    but if there are other (or at least one additional) 'de' installation the user has to pick which one to use
    the default is the newer one but if an older download finishes, a dialog is asking the user
*/

#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include <QList>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QObject>
#include <QDir>
#include <QSettings>

#include "localarchive.h"
#include "partialarchive.h"
#include "downloadablearchive.h"

#include "defines.h"


/*! manages different wikipedia archives (also called "dumps" in the past)
** - this class should be instantiated only once
** - manages torrent downloads of archives
** - sets the default archives for each language
**   one may have installed several 'de' archives, but only
**   one can be used at a time.
** - manages archives, adds(downloads or removes them)
*/
class ArchiveManager : public QObject {
Q_OBJECT

    QHash<ArchiveID, Archive *> archives;
    /*! subset of archives */
    QHash<QString, LocalArchive *> defaultLocalArchives;

    QNetworkAccessManager netManager;

    void restoreLocalAndPartialArchives(QSettings &settings);
    bool addArchiveInternal(Archive *a);
    bool addArchiveAndStoreInSettings(Archive *a);

private slots:
    void networkFinished(QNetworkReply *reply);
    void updateDefaultLocalArchives(const QList<Archive *> &archives);


public:
    explicit ArchiveManager(QObject* parent);

    const QDir getArchivesBaseDir() const {
        return QDir("/tmp/");
//        return QDir("/home/user/MyDocs/wikipedia/");
    }

    /*! takes ownership of the object if it is added */
    bool addArchive(Archive *archive);

    LocalArchive *getLocalArchive(const QString language, const QString date=QString()) const;
    LocalArchive *getRandomLocalArchive() const;
    const QHash<QString, LocalArchive *> getDefaultLocalArchives() const;
    bool hasLanguage(const QString language) const;

    const QHash<ArchiveID, Archive *> &getArchives() const { return archives; }

    /*! used for type transitions of one archive */
    void exchangeArchives(DownloadableArchive *from, PartialArchive *to);
    void exchangeArchives(PartialArchive *from, LocalArchive *to);

signals:
    /* these two are not emitted if archivesExchanged is emitted */
    void defaultLocalArchivesChanged(const QList<LocalArchive *> &archives);
    void archivesChanged(const QList<Archive *> &archives);

    void archivesExchanged(DownloadableArchive *from, PartialArchive *to);
    void archivesExchanged(PartialArchive *from, LocalArchive *to);

public slots:
    void updateRemoteArchives();
    void setDownloadsPaused(bool value);
};

#endif
