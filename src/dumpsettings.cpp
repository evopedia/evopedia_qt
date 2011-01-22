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

#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QStandardItemModel>

#include "dumpsettings.h"
#include "ui_dumpSettings.h"
#include "archivemanager.h"
#include "evopedia.h"
#include "evopediaapplication.h"

DumpSettings::DumpSettings(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::DumpSettings)
{
    ui->setupUi(this);

#if defined(Q_OS_SYMBIAN)
    QAction *closeAction = new QAction(tr("Back"), ui->menuMenu);
    connect(closeAction, SIGNAL(triggered()), SLOT(close()));
    ui->menuMenu->addAction(closeAction);
    ui->actionCompact_Layout->setChecked(true);
#endif

    evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();

    connect(ui->actionRefresh_archive_list, SIGNAL(triggered()), evopedia->getArchiveManager(), SLOT(updateRemoteArchives()));

    connect(evopedia->getArchiveManager(),
            SIGNAL(archivesChanged(QList<Archive*>)),
            ui->archiveList,
            SLOT(updateArchives(QList<Archive*>)));
    connect(evopedia->getArchiveManager(),
            SIGNAL(archivesExchanged(DownloadableArchive*,PartialArchive*)),
            ui->archiveList,
            SLOT(exchangeArchives(DownloadableArchive*,PartialArchive*)));
    connect(evopedia->getArchiveManager(),
            SIGNAL(archivesExchanged(PartialArchive*,LocalArchive*)),
            ui->archiveList,
            SLOT(exchangeArchives(PartialArchive*,LocalArchive*)));
    ui->archiveList->updateArchives(evopedia->getArchiveManager()->getArchives().values());
}

DumpSettings::~DumpSettings()
{
    delete ui;
}

void DumpSettings::on_actionManually_add_archive_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Archive Directory"),
                                                    QString(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        LocalArchive *archive = new LocalArchive(dir);
        if (archive->isReadable()) {
            if (evopedia->getArchiveManager()->addArchive(archive)) {
                return; /* ownership transferred */
            } else {
                QMessageBox::critical(NULL, tr("Error"),
                                      tr("Archive '%1 %2' already installed.")
                                      .arg(archive->getLanguage()).arg(archive->getDate()));
                delete archive;
            }
        } else {
            delete archive;

            QStringList torrents = QDir(dir).entryList(QStringList() << "wikipedia_*_*-*-*.torrent");
            if (!torrents.isEmpty()) {
                QString torrent = torrents[0];
                QStringList parts = torrent.split('_');
                QString lang = parts[1];
                QString date = parts[2].mid(0, 10);

                PartialArchive *pa = new PartialArchive(lang, date, QString(),
                                                        torrent, dir);
                if (evopedia->getArchiveManager()->addArchive(pa)) {
                    return; /* ownership transferred */
                } else {
                    delete pa;
                    QMessageBox::critical(NULL, tr("Error"),
                                          tr("Archive '%1 %2' already installed.")
                                          .arg(lang, date));
                }
            }
            QMessageBox::critical(NULL, tr("Error"),
                                  tr("Directory '%1' does not contain a valid evopedia archive:\n%2")
                                  .arg(dir).arg(archive->getErrorMessage()));
        }
    }
}

void DumpSettings::on_actionChange_Default_Archive_Dir_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Change Default Download Directory"),
                                                            evopedia->getArchiveManager()->getArchivesBaseDir(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
        evopedia->getArchiveManager()->setArchivesBaseDir(dir);
}

void DumpSettings::on_actionCompact_Layout_toggled(bool value)
{
    ui->archiveList->setCompactLayout(value);
    ui->archiveList->updateArchives(evopedia->getArchiveManager()->getArchives().values());
}
