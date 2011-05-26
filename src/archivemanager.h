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

    QString archivesBaseDir;

    void restoreLocalAndPartialArchives(QSettings &settings);
    bool addArchiveInternal(Archive *a);
    bool addArchiveAndStoreInSettings(Archive *a);

private slots:
    void handleNetworkFinished(QNetworkReply *reply);
    void updateDefaultLocalArchivesUponExchange(PartialArchive *from, LocalArchive *to);
    void updateDefaultLocalArchives(QList<Archive *> archives);


public:
    explicit ArchiveManager(QObject* parent);

    const QString getArchivesBaseDir() const {
        return archivesBaseDir;
    }
    void setArchivesBaseDir(QString dir);

    /*! takes ownership of the object if it is added */
    bool addArchive(Archive *archive);

    LocalArchive *getLocalArchive(const QString language, const QString date=QString()) const;
    LocalArchive *getRandomLocalArchive() const;
    const QHash<QString, LocalArchive *> getDefaultLocalArchives() const;
    bool hasLanguage(const QString language) const;
    bool isDefaultForLanguage(const LocalArchive *archive) const {
        return getLocalArchive(archive->getLanguage()) == archive;
    }

    const QHash<ArchiveID, Archive *> &getArchives() const { return archives; }

    /*! used for type transitions of one archive */
    void exchangeArchives(DownloadableArchive *from, PartialArchive *to);
    void exchangeArchives(PartialArchive *from, LocalArchive *to);

signals:
    void defaultLocalArchivesChanged(QList<LocalArchive *> archives);

    /* this one is not emitted if archivesExchanged is emitted */
    void archivesChanged(QList<Archive *> archives);
    void archivesExchanged(DownloadableArchive *from, PartialArchive *to);
    void archivesExchanged(PartialArchive *from, LocalArchive *to);

public slots:
    void updateRemoteArchives();
};

#endif
