#include "downloadablearchive.h"

#include <QFileDialog>

#include "evopediaapplication.h"

DownloadableArchive::DownloadableArchive(const QString &language, const QString &date,
                                         const QUrl &url, const QString &size, QObject *parent) :
    Archive(parent), url(url), size(size)
{
    this->language = language;
    this->date = date;
}

QString DownloadableArchive::askAndCreateDownloadDirectory()
{
    ArchiveManager *am((static_cast<EvopediaApplication *>(qApp))->evopedia()->getArchiveManager());
    const QDir d;
    QString baseDir = am->getArchivesBaseDir();

    if (baseDir.isEmpty()) {
        baseDir = QFileDialog::getExistingDirectory(0, tr("Select Base Download Directory For Archives"),
                                                        QString(), QFileDialog::ShowDirsOnly);
        if (baseDir.isEmpty())
            return QString();
        else
            am->setArchivesBaseDir(baseDir);
    }

    QString downloadDirectory = QDir(baseDir).absolutePath() + "/" + QString("wikipedia_%1").arg(language);

    if (!QDir(downloadDirectory).exists()) {
        if (!QDir().mkpath(downloadDirectory)) {
            QMessageBox::critical(0, tr("Error Downloading Torrent"),
                                  tr("Unable to create directory %1.")
                                  .arg(downloadDirectory));
            return QString();
        }
    }

    return downloadDirectory;
}

bool DownloadableArchive::startDownload()
{
    downloadDirectory = askAndCreateDownloadDirectory();

    /* TODO2 sanity check for language and date? */
    torrentFile = QString("wikipedia_%1_%2.torrent").arg(language, date);

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QObject::connect(manager, SIGNAL(finished(QNetworkReply* )),
                     this, SLOT(torrentDownloadFinished(QNetworkReply* )));
    manager->get(QNetworkRequest(url));

    /* TODO1 indicator (progress bar, throbber) while file is downloaded */
    /* TODO1 it could be possible that we have to move this code to
       PartialArchive in order to achive that */
    return true;
}

/* TODO1 download error handler */
void DownloadableArchive::torrentDownloadFinished(QNetworkReply* reply) {
    QFile f(downloadDirectory + "/" + torrentFile);
    f.open(QIODevice::WriteOnly);
    f.write(reply->readAll());
    f.close();

    ArchiveManager *am((static_cast<EvopediaApplication *>(qApp))->evopedia()->getArchiveManager());
    PartialArchive *a = new PartialArchive(language, date, size,
                                           torrentFile, downloadDirectory);
    am->exchangeArchives(this, a);

    a->startDownload();
}
