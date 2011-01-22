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

#ifndef ARCHIVEDETAILSDIALOG_H
#define ARCHIVEDETAILSDIALOG_H

#include <QDialog>

#include "downloadablearchive.h"
#include "partialarchive.h"
#include "localarchive.h"

namespace Ui {
    class ArchiveDetailsDialog;
}

class ArchiveDetailsDialog : public QDialog
{
    Q_OBJECT

    void fillBasicInfo(Archive *a, const QString &size, const QString &state);
public:
    explicit ArchiveDetailsDialog(DownloadableArchive *a, QWidget *parent = 0);
    explicit ArchiveDetailsDialog(PartialArchive *a, QWidget *parent = 0);
    explicit ArchiveDetailsDialog(LocalArchive *a, QWidget *parent = 0);
    ~ArchiveDetailsDialog();

private:
    Ui::ArchiveDetailsDialog *ui;
};

#endif // ARCHIVEDETAILSDIALOG_H
