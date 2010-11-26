#include "archivedetailsdialog.h"
#include "ui_archivedetailsdialog.h"

#include <QDialogButtonBox>
#include <QProgressBar>
#include <QPushButton>

ArchiveDetailsDialog::ArchiveDetailsDialog(DownloadableArchive *a, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ArchiveDetailsDialog)
{
    ui->setupUi(this);

    QString size = a->getSize(); /* in Bytes */
    fillBasicInfo(a,
                  tr("%1 MB").arg(size.left(size.length() - 6)),
                  tr("not downloaded"));

    ui->buttonBox->addButton(tr("Download Now"), QDialogButtonBox::AcceptRole);
}

ArchiveDetailsDialog::ArchiveDetailsDialog(PartialArchive *a, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ArchiveDetailsDialog)
{
    ui->setupUi(this);

    fillBasicInfo(a,
                  a->getSizeMB(),
                  QString());

    QLabel *value;

    int r = ui->formLayout->rowCount() - 1;

    value = new QLabel(this);
    ui->formLayout->insertRow(r++, tr("Peers:"), value);
    connect(a, SIGNAL(peerInfoUpdated(QString)), value, SLOT(setText(QString)));

    value = new QLabel(this);
    ui->formLayout->insertRow(r++, tr("Speed:"), value);
    connect(a, SIGNAL(speedTextUpdated(QString)), value, SLOT(setText(QString)));

    QProgressBar *pbar = new QProgressBar(this);
    pbar->setMinimum(0);
    pbar->setMaximum(100);
    connect(a, SIGNAL(progressUpdated(int)), pbar, SLOT(setValue(int)));

    ui->formLayout->insertRow(r++, pbar);

    connect(a, SIGNAL(statusTextUpdated(QString)), ui->label_state, SLOT(setText(QString)));

    a->emitStatusEvents();
}

ArchiveDetailsDialog::ArchiveDetailsDialog(LocalArchive *a, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ArchiveDetailsDialog)
{

}

ArchiveDetailsDialog::~ArchiveDetailsDialog()
{
    delete ui;
}

void ArchiveDetailsDialog::fillBasicInfo(Archive *a, const QString &size, const QString &state)
{
    ui->label_langdate->setText(QString("%1 - %2").arg(a->getLanguage(), a->getDate()));
    ui->label_size->setText(size);
    ui->label_state->setText(state);
}
