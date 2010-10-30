#include "partialarchive.h"

#include <QMessageBox>
#include <QDir>

#include "evopediaapplication.h"
#include "archivemanager.h"
#include "localarchive.h"
#include "torrent/torrentclient.h"
#include "torrent/ratecontroller.h"

PartialArchive::PartialArchive(const QString &language, const QString &date,
                               const QString &size,
                               const QString &torrentFile, const QString &dir, QObject *parent) :
    Archive(parent), size(size), torrentFile(torrentFile), dir(dir),
    uploadRate(0), downloadRate(0), torrentClient(0)
{
    this->language = language;
    this->date = date;
}

PartialArchive *PartialArchive::restoreArchive(QSettings &settings, QObject *parent)
{
    QStringList groupParts(settings.group().split('_'));
    if (groupParts.length() < 3) return 0;

    QString dir(settings.value("data_directory").toString());
    if (!QDir(dir).exists()) return 0;

    QString torrent_file(settings.value("torrent_file").toString());
    if (!QDir(dir).exists(torrent_file)) return 0;

    return new PartialArchive(groupParts[1], groupParts[2],
                              settings.value("download_size", "0").toString(),
                              torrent_file, dir, parent);
}

void PartialArchive::saveToSettings(QSettings &settings) const
{
    settings.beginGroup(QString("dump_%1_%2").arg(language, date));
    settings.setValue("complete", false);
    settings.setValue("data_directory", dir);
    settings.setValue("download_size", size);
    settings.setValue("torrent_file", torrentFile);
    settings.endGroup();

    settings.sync();
}

void PartialArchive::togglePauseDownload()
{
    if (!torrentClient) {
        startDownload();
    } else {
        torrentClient->setPaused(isDownloading());
    }
}

QString PartialArchive::getSizeMB() const
{
    if (size.isEmpty())
        return size;
    else
        return size.left(size.length() - 6) + tr(" MB");
}

bool PartialArchive::validate(QString &ret)
{
    // if already done, we would find a file calles archivechecked.true
    // return true
    // else
    // if (torrent_validation()) {
    //  touch archivechecked.true
    //  return true;
    // }
    // return false;
    return false;
}

void PartialArchive::startDownload()
{
    /* TODO1 automatically stop downloading depending on battery state */
    /* TODO1 automatically stop downloading depending on free space on filesystem */
    // FIXME1 parse METADATA from the dumps webpage must match 'lang' and 'date' with the contents of the torrent
    if (!torrentClient)
        torrentClient = new TorrentClient(this);

    // setting a rate is important: not doing so will result in no seeds reported & silent fails
    /* TODO0 set a reasonable rate for each device */
    int rate = 1000*1000*10; // 10mb/s
    RateController::instance()->setUploadLimit(rate);
    RateController::instance()->setDownloadLimit(rate);

    // Setup the client connections
    connect(torrentClient, SIGNAL(stateChanged(TorrentClient::State)),
           this, SLOT(updateState(TorrentClient::State)));
    connect(torrentClient, SIGNAL(peerInfoUpdated()),
           this, SLOT(updatePeerInfo()));
    connect(torrentClient, SIGNAL(progressUpdated(int)),
           this, SIGNAL(progressUpdated(int)));
    connect(torrentClient, SIGNAL(downloadRateUpdated(int)),
           this, SLOT(updateDownloadRate(int)));
    connect(torrentClient, SIGNAL(uploadRateUpdated(int)),
           this, SLOT(updateUploadRate(int)));
    connect(torrentClient, SIGNAL(stopped()),
           this, SLOT(torrentStopped()));
    connect(torrentClient, SIGNAL(error(TorrentClient::Error)),
           this, SLOT(torrentError(TorrentClient::Error)));

    if (!torrentClient->setTorrent(dir + "/" + torrentFile)) {
            QMessageBox::warning(NULL, tr("Error"),
                    tr("The torrent file %1/%2 cannot not be opened/resumed.").arg(dir, torrentFile));
            delete torrentClient;
            torrentClient = 0;
            return;
     }

     torrentClient->setDestinationFolder(dir);

     torrentClient->start();
}

bool PartialArchive::isDownloading() const
{
    return (torrentClient != 0 && torrentClient->state() != TorrentClient::Paused);
}

void PartialArchive::pauseDownload()
{
    torrentClient->setPaused(true);
}

void PartialArchive::updateState(TorrentClient::State s)
{
    QString statusText;
    switch (s) {
    case TorrentClient::Idle: statusText = tr("idle"); break;
    case TorrentClient::Paused: statusText = tr("paused"); break;
    case TorrentClient::Stopping: statusText = tr("stopping"); break;
    case TorrentClient::Preparing: statusText = tr("preparing"); break;
    case TorrentClient::Searching: statusText = tr("searching"); break;
    case TorrentClient::Connecting: statusText = tr("connecting"); break;
    case TorrentClient::WarmingUp: statusText = tr("warming up"); break;
    case TorrentClient::Downloading: statusText = tr("downloading"); break;
    case TorrentClient::Endgame: statusText = tr("endgame"); break;
    case TorrentClient::Seeding: statusText = tr("seeding"); break;
    }

    emit statusTextUpdated(statusText);

    if (s == TorrentClient::Paused)
        emit downloadPaused();
    else
        emit downloadStarted();

    if (s == TorrentClient::Seeding)
        changeToLocalArchive();
}

void PartialArchive::updatePeerInfo()
{
    emit peerInfoUpdated(tr("%n seed(s), ", "", torrentClient->seedCount()) +
                         tr("%n peer(s)", "", torrentClient->connectedPeerCount()));
}

void PartialArchive::updateDownloadRate(int rate)
{
    downloadRate = rate / 1024.0;
    emit speedTextUpdated(tr("%1/%2 kB/s down/up")
                          .arg(downloadRate, 0, 'f', 2)
                          .arg(uploadRate, 0, 'f', 2));
}

void PartialArchive::updateUploadRate(int rate)
{
    uploadRate = rate / 1024.0;
    emit speedTextUpdated(tr("%1/%2 kB/s down/up")
                          .arg(downloadRate, 0, 'f', 2)
                          .arg(uploadRate, 0, 'f', 2));
}

void PartialArchive::torrentStopped()
{
    qDebug() << __PRETTY_FUNCTION__;
    //FIXME0 handle this
    //m_torrentclient->deleteLater();
}

void PartialArchive::torrentError(TorrentClient::Error)
{
    //FIXME0 handle this
    qDebug() << __PRETTY_FUNCTION__ << torrentClient->errorString();
}

void PartialArchive::changeToLocalArchive()
{
    QString localArchiveDir;
    if (QDir(dir).exists("metadata.txt")) {
        localArchiveDir = dir;
    } else {
        QStringList subdirs = QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (subdirs.isEmpty()) {
            localArchiveDir = dir; /* will fail */
        } else {
            /* there should only be one */
            localArchiveDir = dir + "/" + subdirs[0];
        }
    }

    LocalArchive *a = new LocalArchive(localArchiveDir);
    if (!a->isReadable()) {
        QString err(a->getErrorMessage());
        QMessageBox::critical(0, "Error", tr("Archive downloaded completely but it "
                                                  "is not valid and cannot be used (%1).").arg(err));
        delete a;
    } else {
        ArchiveManager *am((static_cast<EvopediaApplication *>(qApp))->evopedia()->getArchiveManager());
        am->exchangeArchives(this, a);
        /* TODO0 "this" is destroyed after that call. What happens to the torrent client? */
    }

}
