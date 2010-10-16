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

    connect(ui->addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
    connect(ui->refreshButton, SIGNAL(clicked()), evopedia->getArchiveManager(), SLOT(updateRemoteArchives()));

    //ui->treeView->setModel(evopedia->archivemanager->model());
    //ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(evopedia->getArchiveManager(),
            SIGNAL(archivesChanged(QList<Archive*>)),
            ui->archiveList,
            SLOT(updateArchives(QList<Archive*>)));
    connect(evopedia->getArchiveManager(),
            SIGNAL(archivesExchanged(DownloadableArchive*,PartialArchive*)),
            ui->archiveList,
            SLOT(exchangeArchives(DownloadableArchive*,PartialArchive*)));
    ui->archiveList->updateArchives(evopedia->getArchiveManager()->getArchives().values());

    show();
}

DumpSettings::~DumpSettings()
{
    delete ui;
}

void DumpSettings::addButtonClicked()
{
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
            QMessageBox::critical(NULL, tr("Error"),
                                  tr("Directory '%1' does not contain a valid evopedia archive:\n '%2'")
                                  .arg(dir).arg(archive->getErrorMessage()));
        }
        delete archive;
    }
}

