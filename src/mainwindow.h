#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "storagebackend.h"
#include "titlelistmodel.h"
#include "evopedia.h"
#include "mapwindow.h"
#include "ui_dumpSettings.h"


namespace Ui {
    class Evopedia;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionDeny_toggled(bool v);
    void on_actionAllow_toggled(bool v);
    void on_actionAuto_toggled(bool v);
    void on_actionConfigure_Dumps_triggered();
    void on_actionMap_triggered();
    void on_actionAbout_triggered();
    void on_languageChooser_currentIndexChanged(const QString &text);
    void on_listView_activated(QModelIndex index);
    void on_searchField_textChanged(const QString &text);
    void mapViewRequested(qreal lat, qreal lon, uint zoom);
    void backendsChanged(const QList<StorageBackend *>backends);

    void refreshSearchResults();

private:
    void showMapWindow();

    Ui::Evopedia *ui;

    TitleListModel *titleListModel;
    MapWindow *mapWindow;
};

#endif // MAINWINDOW_H
