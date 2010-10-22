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

    evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();

    connect(ui->actionRefresh_archive_list, SIGNAL(triggered()), evopedia->getArchiveManager(), SLOT(updateRemoteArchives()));
    connect(ui->actionPause_all_downloads, SIGNAL(toggled(bool)), evopedia->getArchiveManager(), SLOT(setDownloadsPaused(bool)));

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

    /* TODO show some message explaining how to use this. mention menu. only for the first time? */

    show();
}

DumpSettings::~DumpSettings()
{
    delete ui;
}

void DumpSettings::on_actionManually_add_archive_triggered()
{
    /* TODO creates a rather complicated dialog. */

    QFileDialog dialog(this, tr("Open Dump Directory"), QString());
    dialog.setFileMode(QFileDialog::DirectoryOnly);

    if (dialog.exec()) {
        QString dir = dialog.selectedFiles().first();
        LocalArchive *archive = new LocalArchive(dir);
        if (archive->isReadable()) {
            if (evopedia->getArchiveManager()->addArchive(archive))
                return; /* ownership transferred */
            else
                QMessageBox::critical(NULL, tr("Error"),
                                      tr("Archive '%1 %2' already installed.")
                                      .arg(archive->getLanguage()).arg(archive->getDate()));
        } else {
            QStringList torrents = QDir(dir).entryList(QStringList() << "wikipedia_*_*-*-*.torrent");
            if (!torrents.isEmpty()) {
                QString torrent = torrents[0];
                QStringList parts = torrent.split('_');
                QString lang = parts[1];
                QString date = parts[2].mid(0, 10);

                /* TODO perhaps ask before doing that? */
                PartialArchive *pa = new PartialArchive(lang, date, "unknown",
                                                        dir + "/" + torrent, dir);
            }
            QMessageBox::critical(NULL, tr("Error"),
                                  tr("Directory '%1' does not contain a valid evopedia archive:\n '%2'")
                                  .arg(dir).arg(archive->getErrorMessage()));
        }
        delete archive;
    }
}

