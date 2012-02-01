/*
 * evopedia: An offline Wikipedia reader.
 *
 * Copyright (C) 2010-2011 evopedia developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
    ArchiveManager *am((static_cast<EvopediaApplication *>(QCoreApplication::instance()))->evopedia()->getArchiveManager());
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

    ArchiveManager *am((static_cast<EvopediaApplication *>(QCoreApplication::instance()))->evopedia()->getArchiveManager());
    PartialArchive *a = new PartialArchive(language, date, size,
                                           torrentFile, downloadDirectory);
    am->exchangeArchives(this, a);

    a->startDownload();
}
