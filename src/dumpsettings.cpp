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
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)),
               this, SLOT(showContextMenu(const QPoint &)));

    connect(ui->addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
    connect(ui->refreshButton, SIGNAL(clicked()), SLOT(refreshButtonClicked()));

    evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();

    ui->treeView->setModel(evopedia->archivemanager->model());

    ui->treeView->setColumnWidth(0, 70); //fm.width("lang");
    ui->treeView->setColumnWidth(1, 170);//fm.width("2010-07-07"));
    ui->treeView->setColumnWidth(2, 100);//fm.width("14 gb"));
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

void DumpSettings::showContextMenu(const QPoint &position) {
     //FIXME create a QMenu per ArchiveItem reflecting it's type

    //QList<QAction *> actions;
    //if (view->indexAt(position).isValid()) {
     //   actions.append(openAction);
    //}
    //if (actions.count() > 0)
      //  QMenu::exec(actions, views->mapToGlobal(position));
}

