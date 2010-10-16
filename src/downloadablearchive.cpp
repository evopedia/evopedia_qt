#include "downloadablearchive.h"

#include "evopediaapplication.h"

DownloadableArchive::DownloadableArchive(const QString &language, const QString &date,
                                         const QUrl &url, const QString &size, QObject *parent) :
    Archive(parent), url(url), size(size)
{
    this->language = language;
    this->date = date;
}

bool DownloadableArchive::startDownload()
{
    ArchiveManager *am((static_cast<EvopediaApplication *>(qApp))->evopedia()->getArchiveManager());
    const QDir baseDir(am->getArchivesBaseDir());

    /* TODO sanity check for language and date? */
    downloadDirectory = baseDir.absolutePath() + "/" + QString("wikipedia_%1").arg(language);
    torrentFile = QString("wikipedia_%1_%2.torrent").arg(language, date);

    if (!QDir(downloadDirectory).exists()) {
        if (!QDir().mkpath(downloadDirectory)) {
            /* TODO error message */
            return false;
        }
    }

    QFile f(downloadDirectory + "/" + torrentFile);
    if (f.exists()) {
        /* rather remove it, could be corrupted */
        if (!f.remove()) {
            /* TODO error? */
            // return false;
        }
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply* )),
                     this, SLOT(torrentDownloadFinished(QNetworkReply* )));
    manager->get(QNetworkRequest(url));

    /* TODO indicator (progress bar, throbber) while file is downloaded */
    return true;
}

void DownloadableArchive::torrentDownloadFinished(QNetworkReply* reply) {
    QFile f(downloadDirectory + "/" + torrentFile);
    f.open(QIODevice::WriteOnly);
    f.write(reply->readAll());
    f.close();

    ArchiveManager *am((static_cast<EvopediaApplication *>(qApp))->evopedia()->getArchiveManager());
    PartialArchive *a = new PartialArchive(language, date, url, size,
                                           torrentFile, downloadDirectory);
    am->exchangeArchives(this, a);

    a->startDownload();
}
