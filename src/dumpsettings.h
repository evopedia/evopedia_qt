#ifndef DUMPSETTINGS_H
#define DUMPSETTINGS_H

#include <QDialog>
#include <QListWidget>

#include "storagebackend.h"

namespace Ui {
    class DumpSettings;
}

class DumpSettings : public QDialog
{
    Q_OBJECT
public:
    explicit DumpSettings(QWidget *parent = 0);
    ~DumpSettings();

signals:

private slots:
    void on_addDump_clicked();
    void on_removeDump_clicked();
    void on_dumpList_itemSelectionChanged();
    void backendsChanged(const QList<StorageBackend *>backends);

private:

    Ui::DumpSettings *ui;
};

#endif // DUMPSETTINGS_H
