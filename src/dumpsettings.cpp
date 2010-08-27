#include "dumpsettings.h"

#include "ui_dumpSettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMutableListIterator>

#include "evopedia.h"
#include "evopediaapplication.h"


DumpSettings::DumpSettings(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::DumpSettings)
{
    ui->setupUi(this);

    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    backendsChanged(evopedia->getBackends());
    connect(evopedia, SIGNAL(backendsChanged(const QList<StorageBackend*>)),
            SLOT(backendsChanged(const QList<StorageBackend*>)));

    connect(&netManager, SIGNAL(finished(QNetworkReply*)), SLOT(networkFinished(QNetworkReply*)));
}

DumpSettings::~DumpSettings()
{
    delete ui;
}

void DumpSettings::on_actionRefresh_triggered()
{
    netManager.get(QNetworkRequest(QUrl("http://dumpathome.evopedia.info/dumps/finished")));
}

void DumpSettings::networkFinished(QNetworkReply *reply)
{
    QMutableListIterator<Archive> i(archives);
    while (i.hasNext())
        if (i.next().state == Archive::OnServer)
            i.remove();

    QString data = QString::fromUtf8(reply->readAll().constData());

    QRegExp rx("wikipedia_([a-z]*)_([0-9-]*)\\.([a-z.]*)\">.*([0-9.]* [GKB]B)");
    rx.setMinimal(true);

    for (int pos = 0; (pos = data.indexOf(rx, pos + 1)) > 0;) {
        /* TODO skip if we are currently downloading this archive
           or if it is already installed! */
        Archive a;
        a.language = rx.cap(1);
        a.date = rx.cap(2);
        /* 3 is file type */
        a.size = rx.cap(4);
        /* TODO for testing */
        if (archives.length() % 4 == 0)
            a.state = Archive::Downloading;
        else
            a.state = Archive::OnServer;
        a.articleCount = "";

        archives += a;
    }

    qSort(archives);

    updateView();
}

void DumpSettings::updateView()
{
    ui->dumpList->clear();
    /* TODO respect some "only display installed archives
       or archives that are downloaded" setting
       and filter accordingly */
    {
        QListWidgetItem *item = new QListWidgetItem;
        QPushButton *button = new QPushButton(QIcon::fromTheme("general_add"), "add manually downloaded archive");
        connect(button, SIGNAL(clicked()), SLOT(manualAddClicked()));
        item->setSizeHint(button->sizeHint());
        ui->dumpList->addItem(item);
        ui->dumpList->setItemWidget(item, button);
    }
    /* TODO add a "manually add archive" button into the list */
    foreach (Archive a, archives) {
        QListWidgetItem *item = new QListWidgetItem;
        /* TODO maemo styles have grey second lines */
        QString text = QString("<b>Wikipedia %1 (%2)</b><br/><small>%3</small>")
                       .arg(a.language, a.date, a.size);
        /* TODO show article count somewhere */
        QLabel *label = new QLabel(text);
        label->setTextFormat(Qt::RichText);

        QWidget *widget;
        if (a.state == Archive::Downloading) {
            widget = new QWidget;
            QVBoxLayout *layout = new QVBoxLayout;
            QProgressBar *progressBar = new QProgressBar;
            layout->addWidget(label);
            layout->addWidget(progressBar);
            widget->setLayout(layout);
        } else {
            widget = label;
        }
        item->setSizeHint(widget->sizeHint());
        ui->dumpList->addItem(item);
        ui->dumpList->setItemWidget(item, widget);
    }
}


void DumpSettings::manualAddClicked()
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

#if 0
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
#endif

void DumpSettings::on_dumpList_itemSelectionChanged()
{
    //ui->removeDump->setEnabled(!ui->dumpList->selectedItems().isEmpty());
}

void DumpSettings::backendsChanged(const QList<StorageBackend *>backends)
{
    QMutableListIterator<Archive> i(archives);
    while (i.hasNext())
        if (i.next().state == Archive::Installed)
            i.remove();

    QListWidget *dumpList = ui->dumpList;
    dumpList->clear();
    foreach (StorageBackend *b, backends) {
        Archive a;
        a.articleCount = b->getNumArticles();
        a.date = b->getDate();
        a.language = b->getLanguage();
        a.size = "";
        a.state = Archive::Installed;
        archives += a;
    }

    qSort(archives);
    updateView();
}
