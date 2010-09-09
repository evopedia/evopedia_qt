#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMutableListIterator>
#include <QListWidgetItem>

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

    ui->dumpTree->setColumnWidth(0, 70); //fm.width("lang");
    ui->dumpTree->setColumnWidth(1, 170);//fm.width("2010-07-07"));
    ui->dumpTree->setColumnWidth(2, 100);//fm.width("14 gb"));
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
    /* TODO2 already check for dump in the file chooser */
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Dump Directory"),
                                                     QString(),
                                                     QFileDialog::ShowDirsOnly);
    QString ret; // return error message, if any
    if (!evopedia->archivemanager->addArchive(dir, ret)) {
        QMessageBox::critical(NULL, tr("Error"),
                              tr("Directory %1 does not contain a valid evopedia dump (%2).")
                              .arg(dir).arg(ret));
    }
}

