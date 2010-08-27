#ifndef DUMPSETTINGS_H
#define DUMPSETTINGS_H

#include <QDialog>
#include <QListWidget>
#include <QList>
#include <QNetworkAccessManager>
#include <QMainWindow>

#include "storagebackend.h"
#include "archive.h"

namespace Ui {
    class DumpSettings;
}

class DumpSettings : public QMainWindow
{
    Q_OBJECT
public:
    explicit DumpSettings(QWidget *parent = 0);
    ~DumpSettings();

signals:

protected slots:
    void updateView();

private slots:
    void on_actionRefresh_triggered();
    void manualAddClicked();
    void on_dumpList_itemSelectionChanged();
    void backendsChanged(const QList<StorageBackend *>backends);
    void networkFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager netManager;
    QList<Archive> archives;
    Ui::DumpSettings *ui;
};

#endif // DUMPSETTINGS_H
