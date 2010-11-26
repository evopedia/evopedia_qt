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
