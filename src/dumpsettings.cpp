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

    connect(ui->addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
    connect(ui->refreshButton, SIGNAL(clicked()), SLOT(refreshButtonClicked()));

    evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();

    ui->treeView->setModel(evopedia->archivemanager->model());
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->treeView->setColumnWidth(0, 200);//fm.width("lang/2010-07-07");
    ui->treeView->setColumnWidth(1, 120);//fm.width(""));
    ui->treeView->setColumnWidth(2, 100);//fm.width(""));
    ui->treeView->setColumnWidth(3, 100);//fm.width(""));
    show();
}

DumpSettings::~DumpSettings()
{
    delete ui;
}

void DumpSettings::refreshButtonClicked()
{
    evopedia->archivemanager->updateRemoteArchives();
}

void DumpSettings::addButtonClicked()
{
    QFileDialog dialog(this, tr("Open Dump Directory"), QString());
    dialog.setFileMode(QFileDialog::DirectoryOnly);

    if (dialog.exec()) {
        QString dir = dialog.selectedFiles().first();
        QString ret; // return error message, if any
        if (!evopedia->archivemanager->addArchive(dir, ret)) {
            QMessageBox::critical(NULL, tr("Error"),
                                  tr("Directory '%1'' does not contain a valid evopedia dump:\n '%2'")
                                  .arg(dir).arg(ret));
        }
    }
}

