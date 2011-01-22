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

#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H

#include <QObject>
#include <QSettings>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QHash>

#include "titleiterator.h"
#include "geotitle.h"

#include "archive.h"

class LocalArchive : public Archive
{
    Q_OBJECT
public:
    LocalArchive(const QString &directory, QObject *parent=0);

    static LocalArchive *restoreArchive(QSettings &settings, QObject *parent=0);

    void saveToSettings(QSettings &settings) const;

    TitleIterator getTitlesWithPrefix(const QString &prefix);
    QList<GeoTitle> getTitlesInCoords(const QRectF &rect, int maxTitles=-1, bool *complete=0);
    const QByteArray getArticle(const QString &title);
    const Title getTitle(const QString &title);
    const QByteArray getArticle(const Title &t);
    const Title getTitleFromPath(const QStringList &pathParts);
    QUrl getOrigUrl(const Title &title) const;
    const QString &getOrigUrl() const { return dumpOrigURL; }
    const QByteArray getMathImage(const QByteArray &hexHash) const;
    const Title getRandomTitle();

    int getNumArticles() const { return dumpNumArticles.toInt(); }
    bool isReadable() const { return readable; }

    const QString &getErrorMessage() const { return errorMessage; }

    const QString &getDirectory() const { return directory; }
    
    static const QString normalize(const QString &str);
private:
    void initializeCoords(QSettings &metadata);
    bool findMathImage(const QByteArray &hexHash, quint32 &pos, quint32 &length) const;
    void getTitlesInCoordsInt(QList<GeoTitle> &list, QFile &titles, QFile &coordFile, qint64 coordFilePos,
                                               const QRectF &targetRect, const QRectF &thisRect,
                                               int maxTitles);
    bool checkExistenceOfDumpfiles();

    const Title getTitleAtOffset(quint32 offset);

    QString errorMessage;

    bool readable;
    const QString directory;
    
    QString titleFile;
    QString mathIndexFile;
    QString mathDataFile;
    QStringList coordinateFiles;
    
    QString dumpOrigURL;
    QString dumpVersion;
    QString dumpNumArticles;
    bool normalizedTitles;


    static const QHash<QChar, QChar> *normalizationMap();
};

#endif // STORAGEBACKEND_H
