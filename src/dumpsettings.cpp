#include "dumpsettings.h"

#include "ui_dumpSettings.h"

#include <QFileDialog>
#include <QMessageBox>

#include "evopedia.h"
#include "evopediaapplication.h"


DumpSettings::DumpSettings(QWidget *parent) :
        QDialog(parent), ui(new Ui::DumpSettings)
{
    ui->setupUi(this);
    ui->removeDump->setEnabled(false);

    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    backendsChanged(evopedia->getBackends());
    connect(evopedia, SIGNAL(backendsChanged(const QList<StorageBackend*>)),
            SLOT(backendsChanged(const QList<StorageBackend*>)));
}

DumpSettings::~DumpSettings()
{
    delete ui;
}

void DumpSettings::on_addDump_clicked()
{
    /* TODO2 already check for dump in the file chooser */
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Dump Directory"),
                                                     QString(),
                                                     QFileDialog::ShowDirsOnly);
    StorageBackend *backend = new StorageBackend(dir, this);
    if (!backend->isReadable()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Directory %1 does not contain a valid evopedia dump (%2).")
                              .arg(dir).arg(backend->getErrorMessage()));
        delete backend;
    } else {
        /* transfers ownership */
        Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
        evopedia->addBackend(backend);
    }
}

void DumpSettings::on_removeDump_clicked()
{
    QList<QListWidgetItem *> selItems = ui->dumpList->selectedItems();
    if (selItems.empty()) return;

    QString text = selItems[0]->text();
    QString language = text.left(text.indexOf(QChar(' ')));

    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    StorageBackend *backend = evopedia->getBackend(language);
    if (backend == 0) return;

    evopedia->removeBackend(backend);
}

void DumpSettings::on_dumpList_itemSelectionChanged()
{
    ui->removeDump->setEnabled(!ui->dumpList->selectedItems().isEmpty());
}

void DumpSettings::backendsChanged(const QList<StorageBackend *>backends)
{
    QListWidget *dumpList = ui->dumpList;
    dumpList->clear();
    foreach (StorageBackend *b, backends) {
        QString label = tr("%1 (%2), %3 articles")
                        .arg(b->getLanguage(), b->getDate())
                        .arg(b->getNumArticles());
        dumpList->addItem(label);
    }
}
