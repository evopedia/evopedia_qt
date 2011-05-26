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

#ifndef ARCHIVELIST_H
#define ARCHIVELIST_H

#include <QTreeWidget>
#include <QHash>
#include <QPair>
#include <QString>
#include <QSignalMapper>

#include "archive.h"
#include "localarchive.h"
#include "partialarchive.h"
#include "downloadablearchive.h"

class ArchiveList : public QTreeWidget
{
    Q_OBJECT

    QSignalMapper *downloadPausedMapper;
    QSignalMapper *downloadStartedMapper;
    QSignalMapper *showDetailsMapper;

    bool compactLayout;

    void fillLocalArchiveItem(LocalArchive *a, QTreeWidgetItem *item);
    void fillPartialArchiveItem(PartialArchive *a, QTreeWidgetItem *item);
    void fillDownloadableArchiveItem(DownloadableArchive *a, QTreeWidgetItem *item);

private slots:

    void itemClickedHandler(QTreeWidgetItem *item, int column);
    void downloadPausedHandler(QWidget *button);
    void downloadStartedHandler(QWidget *button);

    void showDetails(QObject *o);

public:
    explicit ArchiveList(QWidget *parent = 0);

signals:

public slots:
    void updateArchives(QList<Archive *> archives);
    void exchangeArchives(DownloadableArchive *from, PartialArchive *to);
    void exchangeArchives(PartialArchive *from, LocalArchive *to);
    /* view has to be updated manually afterwards */
    void setCompactLayout(bool value);

};

#endif // ARCHIVELIST_H
