#include <QSettings>
#include <QAction>
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "torrentfrontend.h"
#include "archiveitem.h"
#include "torrent/torrentclient.h"
#include "torrent/ratecontroller.h"

TorrentFrontend::TorrentFrontend(ArchiveItem* item, QString language, QString date, QString size, QString workingDir, QString archiveDir, QString torrent, QUrl url) {
    m_archiveitem = item;
    m_torrentclient = NULL;
    m_language = language;
    m_date = date;
    m_size = size;
    m_workingDir = workingDir;
    m_archiveDir = archiveDir;
    m_torrent = torrent;
    m_url = url;
}

void TorrentFrontend::extend() {
    for (int i = 0; i < 2; ++i) {
        QStandardItem *item1_1Col1 = new QStandardItem();
        QStandardItem *item1_1Col2 = new QStandardItem();
        QStandardItem *item1_1Col3 = new QStandardItem();
        QStandardItem *item1_1Col4 = new QStandardItem();

        // Spalten vorbereiten
        QList<QStandardItem*> itemList;
        itemList.append(item1_1Col1);
        itemList.append(item1_1Col2);
        itemList.append(item1_1Col3);
        itemList.append(item1_1Col4);

        foreach(QStandardItem* item, itemList)
            item->setEnabled(false);

        m_archiveitem->appendRow(itemList);
    }
}

void TorrentFrontend::collapse() {
    while (m_archiveitem->rowCount()>0) {
        m_archiveitem->removeRow(0);
    }
}

QMenu* TorrentFrontend::createContextMenu() {
    QMenu* m = new QMenu();

    QAction* startTorrentDownloadAct = new QAction(QIcon(), "start torrent download", this);
    connect(startTorrentDownloadAct, SIGNAL(triggered()), this, SLOT(startTorrentDownload()));
    m->addAction(startTorrentDownloadAct);

    QAction* pauseTorrentAct = new QAction(QIcon(), "pause torrent download", this);
    connect(pauseTorrentAct, SIGNAL(triggered()), this, SLOT(removeEntry()));
    m->addAction(pauseTorrentAct);

    QAction* resumeTorrentAct = new QAction(QIcon(), "resume torrent download", this);
    connect(resumeTorrentAct, SIGNAL(triggered()), this, SLOT(resumeTorrentDownload()));
    m->addAction(resumeTorrentAct);

    QAction* cancelTorrentAndRemoveFilesAct = new QAction(QIcon(), "cancel torrent download & remove files", this);
    connect(cancelTorrentAndRemoveFilesAct, SIGNAL(triggered()), this, SLOT(cancelTorrentDownload()));
    m->addAction(cancelTorrentAndRemoveFilesAct);

    //FIXME start seeding
    //FIXME stop seeding
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
    return false;
}

void TorrentFrontend::startTorrentDownload() {
    //FIXME check if download has already started, don't start twice!
    qDebug() << __PRETTY_FUNCTION__;

    if (!QDir(m_workingDir).exists() || m_workingDir == "") {
        QFileDialog dialog(NULL, tr("Select a directory"), QString());
        dialog.setFileMode(QFileDialog::DirectoryOnly);
        if (dialog.exec()) {
            m_workingDir=dialog.selectedFiles().first();
        } else {
            return;
        }
    }
    qDebug() << m_workingDir;

    QFile f(m_workingDir + "/" + m_torrent);
    if (!m_torrent.isEmpty() && f.exists()) {
        startDownloadViaTorrent();
    } else {
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QObject::connect(manager, SIGNAL(finished(QNetworkReply* )),
                         this, SLOT(torrentDownloadFinished(QNetworkReply* )));
        manager->get(QNetworkRequest(m_url));
        //FIXME 1. set status message
        //FIXME 2. create timeout error
    }
}

/*! when the *.torrent is downloaded successfully, we start the actual download using it */
void TorrentFrontend::torrentDownloadFinished(QNetworkReply* reply) {
    QFile file(m_workingDir + "/" + m_torrent);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();
    startDownloadViaTorrent();
}

void TorrentFrontend::startDownloadViaTorrent() {
    // FIXME parse METADATA from the dumps webpage must match 'lang' and 'date' with the contents of the torrent
    extend();

    m_torrentclient = new TorrentClient(this);

    // setting a rate is important: not doing so will result in no seeds reported & silent fails
    int rate = 1000*1000*10; // 10mb/s
    RateController::instance()->setUploadLimit(rate);
    RateController::instance()->setDownloadLimit(rate);

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

    QString torrentName = m_workingDir + "/" + m_torrent;
    qDebug() << torrentName;
    if (!m_torrentclient->setTorrent(torrentName)) {
            QMessageBox::warning(NULL, tr("Error"),
                             tr("The torrent file %1 cannot not be opened/resumed.").arg(torrentName));
            delete m_torrentclient;
            m_torrentclient = NULL;
            return;
     }

     m_torrentclient->setDestinationFolder(m_workingDir);
     qDebug() << __PRETTY_FUNCTION__ << " destination folder = " << m_workingDir;
     QByteArray resumeState;// = settings.value("resumeState").toByteArray();
     m_torrentclient->setDumpedState(resumeState);

     m_torrentclient->start();

     m_archiveitem->setItemState(ItemState::DownloadingTorrent);
     saveSettings();
}

void TorrentFrontend::resumeTorrentDownload() {
    qDebug() << __PRETTY_FUNCTION__;
}

void TorrentFrontend::pauseTorrentDownload() {
    qDebug() << __PRETTY_FUNCTION__;
}

/*! cancel torrent download and remove all files */
void TorrentFrontend::cancelTorrentDownload() {
    qDebug() << __PRETTY_FUNCTION__;
    //client->disconnect();
    //connect(client, SIGNAL(stopped()), this, SLOT(torrentStopped()));
    //client->stop();
}

void TorrentFrontend::updateState(TorrentClient::State s) {
    qDebug() << __PRETTY_FUNCTION__ << m_torrentclient->stateString();
    if (s == TorrentClient::Endgame || s == TorrentClient::Seeding) {
        //FIXME this should be done differently: use the contents of the torrent (the direcotry) and not the torrent
        m_archiveDir = m_workingDir + "/" + m_torrent.left(m_torrent.length()-8);
        qDebug() << "download done, now using this as a local archive " << m_archiveDir;
        m_archiveitem->setItemState(ItemState::LocalTorrent);
    }
}

void TorrentFrontend::updatePeerInfo() {

    if (m_archiveitem->rowCount() > 0) {
      QStandardItem* i;
      i = m_archiveitem->child(0,0);
      if (i)
          i->setText("seeds/peers");
      i = m_archiveitem->child(0,1);
      if (i)
          i->setText(QString("%1/%2")
                 .arg(m_torrentclient->connectedPeerCount())
                 .arg(m_torrentclient->seedCount()));
    }
}

void TorrentFrontend::updateProgress(int percent) {
    if (m_archiveitem->rowCount() > 0) {
      QStandardItem* i;
      i = m_archiveitem->child(0,2);
      if (i)
          i->setText(QString("%1 \%").arg(percent));
      i = m_archiveitem->child(0,3);
      if (i)
          i->setText(m_torrentclient->stateString());
    }
}

void TorrentFrontend::updateDownloadRate(int rate) {
    if (m_archiveitem->rowCount() > 0) {
      QStandardItem* i;
      i = m_archiveitem->child(1,0);
      if (i)
          i->setText(QString("down/up rate"));
      i = m_archiveitem->child(1,2);
      if (i)
          i->setText(QString("%1 kb/s").arg(rate/1000.0));
    }
}

void TorrentFrontend::updateUploadRate(int rate) {
    if (m_archiveitem->rowCount() > 0) {
      QStandardItem* i;
      i = m_archiveitem->child(1,0);
      if (i)
          i->setText(QString("down/up rate"));
      i = m_archiveitem->child(1,3);
      if (i)
          i->setText(QString("%1 kb/s").arg(rate/1000.0));
    }
}

void TorrentFrontend::torrentStopped() {
    qDebug() << __PRETTY_FUNCTION__;
    //FIXME handle this
    //m_torrentclient->deleteLater();
}

void TorrentFrontend::torrentError(TorrentClient::Error) {
    //FIXME handle this
    qDebug() << __PRETTY_FUNCTION__ << m_torrentclient->errorString();
}

QString TorrentFrontend::language() {
    return m_language;
}

QString TorrentFrontend::date(){
    return m_date;
}

QString TorrentFrontend::workingDir(){
    return m_workingDir;
}

QString TorrentFrontend::archiveDir(){
    return m_archiveDir;
}

QString TorrentFrontend::size(){
    return m_size;
}

QUrl TorrentFrontend::url(){
    return m_url;
}

QString TorrentFrontend::stateString(){
    return "torrent download running...";
}
