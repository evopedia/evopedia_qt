#include <QSettings>
#include <QAction>
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

#include "torrentfrontend.h"
#include "archiveitem.h"
#include "torrent/torrentclient.h"
#include "torrent/ratecontroller.h"

TorrentFrontend::TorrentFrontend(ArchiveItem* item) {
    m_archiveitem = item;
    m_torrentclient = NULL;
}

void TorrentFrontend::extend() {
    for (int i = 0; i < 2; ++i) {
        QStandardItem *item1_1Col1 = new QStandardItem(QString("")); // Spalte 1 vom ChildChild
        QStandardItem *item1_1Col2 = new QStandardItem(QString("item1_1Col2")); // Spalte 2 vom ChildChild
        QStandardItem *item1_1Col3 = new QStandardItem(QString("item1_1Col2")); // Spalte 2 vom ChildChild
        QStandardItem *item1_1Col4 = new QStandardItem(QString("")); // Spalte 3 vom ChildChild

        // Spalten vorbereiten
        QList<QStandardItem*> itemList;
        itemList.append(item1_1Col1);
        itemList.append(item1_1Col2);
        itemList.append(item1_1Col3);
        itemList.append(item1_1Col4);
        m_archiveitem->appendRow(itemList); // append Child
    }
}

QMenu* TorrentFrontend::createContextMenu() {
    QMenu* m = new QMenu();

    /*
    QAction* pauseTorrentAct = new QAction(QIcon(), "pause torrent download", this);
    connect(pauseTorrentAct, SIGNAL(triggered()), this, SLOT(removeEntry()));
    m->addAction(pauseTorrentAct);
    */
    QAction* startTorrentDownloadAct = new QAction(QIcon(), "start torrent download", this);
    connect(startTorrentDownloadAct, SIGNAL(triggered()), this, SLOT(startTorrentDownload()));
    m->addAction(startTorrentDownloadAct);

    /*
    m->addAction("pause torrent download");
    m->addAction("resume torrent download");
    m->addAction("cancel torrent download & remove files");
    */
    return m;
}

void TorrentFrontend::saveSettings() {
    /*
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Unable to store settings."));
        return;
    }
    settings.setValue(QString("dump_%1_%2/data_directory")
                      .arg(m_archiveitem->language(), m_archiveitem->date()), m_archiveitem->dir());
    settings.sync();
    settings.setArrayIndex(i);
    settings.setValue("sourceFileName", jobs.at(i).torrentFileName);
    settings.setValue("destinationFolder", jobs.at(i).destinationDirectory);
    settings.setValue("uploadedBytes", jobs.at(i).client->uploadedBytes());
    settings.setValue("downloadedBytes", jobs.at(i).client->downloadedBytes());
    settings.setValue("resumeState", jobs.at(i).client->dumpedState());
    */
}

void TorrentFrontend::unsaveSettings() {
    /*
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Unable to store settings."));
        return;
    }
    settings.remove(QString("dump_%1_%2/data_directory")
                      .arg(m_archiveitem->language(), m_archiveitem->date()));
    settings.sync();
    */
}

/*! torrent based chunk validation */
bool TorrentFrontend::validate(QString &ret) {
    // if already done, we would find a file calles archivechecked.true
    // return true
    // else
    // if (torrent_validation()) {
    //  touch archivechecked.true
    //  return true;
    // }
    // return false;
    return true;
}

void TorrentFrontend::startTorrentDownload() {
    //FIXME check if download has already started, don't start twice!
    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << m_archiveitem->m_dir;
    if (!QDir(m_archiveitem->m_dir).exists() || m_archiveitem->m_dir == "") {
        QFileDialog dialog(NULL, tr("Select a directory"), QString());
        dialog.setFileMode(QFileDialog::DirectoryOnly);

        if (dialog.exec()) {
            QString dir = dialog.selectedFiles().first();
            /*
            if ) {
                QMessageBox::critical(NULL, tr("Error"),
                                      tr("Directory '%1'' does not contain a valid evopedia dump:\n '%2'")
                                      .arg(dir).arg(ret));
            }
            */
            m_archiveitem->m_dir=dir;
        } else {
            return;
        }
    }
    // download the torrent to m_dir
    qDebug() << __PRETTY_FUNCTION__ << "download here";


    // METADATA from the dumps webpage must match 'lang' and 'date' with the contents of the torrent
    extend();

    m_torrentclient = new TorrentClient(this);
    // Setup the client connections
    connect(m_torrentclient, SIGNAL(stateChanged(TorrentClient::State)),
           this, SLOT(updateState(TorrentClient::State)));
    connect(m_torrentclient, SIGNAL(peerInfoUpdated()),
           this, SLOT(updatePeerInfo()));
    connect(m_torrentclient, SIGNAL(progressUpdated(int)),
           this, SLOT(updateProgress(int)));
    connect(m_torrentclient, SIGNAL(downloadRateUpdated(int)),
           this, SLOT(updateDownloadRate(int)));
    connect(m_torrentclient, SIGNAL(uploadRateUpdated(int)),
           this, SLOT(updateUploadRate(int)));
    connect(m_torrentclient, SIGNAL(stopped()),
           this, SLOT(torrentStopped()));
    connect(m_torrentclient, SIGNAL(error(TorrentClient::Error)),
           this, SLOT(torrentError(TorrentClient::Error)));

    QString torrentName = m_archiveitem->m_dir + "/" + m_archiveitem->m_torrent;
    qDebug() << torrentName;
    if (!m_torrentclient->setTorrent(torrentName)) {
            QMessageBox::warning(NULL, tr("Error"),
                             tr("The torrent file %1 cannot not be opened/resumed.").arg(torrentName));
            delete m_torrentclient;
            m_torrentclient = NULL;
            return;
     }

     m_torrentclient->setDestinationFolder(m_archiveitem->m_dir);
     qDebug() << __PRETTY_FUNCTION__ << " destination folder = " << m_archiveitem->m_dir;
     QByteArray resumeState;// = settings.value("resumeState").toByteArray();
     m_torrentclient->setDumpedState(resumeState);


     m_torrentclient->start();
     qDebug() << __PRETTY_FUNCTION__ << "started";
     saveSettings();
}

void TorrentFrontend::resumeTorrentDownload() {
    qDebug() << __PRETTY_FUNCTION__;
}

void TorrentFrontend::pauseTorrentDownload() {
    qDebug() << __PRETTY_FUNCTION__;
}

void TorrentFrontend::cancelTorrentDownload() {
    qDebug() << __PRETTY_FUNCTION__;
    //client->disconnect();
    //connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    //client->stop();
}

void TorrentFrontend::torrentDownloadFinished() {
    qDebug() << __PRETTY_FUNCTION__;
}





void TorrentFrontend::updateState(TorrentClient::State) {
    qDebug() << __PRETTY_FUNCTION__ << m_torrentclient->stateString();
}

void TorrentFrontend::updatePeerInfo() {
    qDebug() << __PRETTY_FUNCTION__ << "peers" << m_torrentclient->connectedPeerCount()
            << ", seeds: " << m_torrentclient->seedCount();
}

void TorrentFrontend::updateProgress(int percent) {
    qDebug() << __PRETTY_FUNCTION__ << percent << "% " << m_torrentclient->stateString();
}

void TorrentFrontend::updateDownloadRate(int downrate) {
    qDebug() << __PRETTY_FUNCTION__ << downrate << " download rate kb/s";
}

void TorrentFrontend::updateUploadRate(int uprate) {
    qDebug() << __PRETTY_FUNCTION__ << uprate << " upload rate kb/s";
}

void TorrentFrontend::torrentStopped() {
    qDebug() << __PRETTY_FUNCTION__;
    //m_torrentclient->deleteLater();
}

void TorrentFrontend::torrentError(TorrentClient::Error) {
    qDebug() << __PRETTY_FUNCTION__ << m_torrentclient->errorString();
}
