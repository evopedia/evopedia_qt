#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "localarchive.h"
#include "titlelistmodel.h"
#include "evopedia.h"
#include "mapwindow.h"
#include "ui_dumpSettings.h"
#include "dumpsettings.h"


namespace Ui {
    class Evopedia;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

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
    void backendsChanged(QList<LocalArchive *>backends);

    void refreshSearchResults();

private:
    void showMapWindow();

    Ui::Evopedia *ui;

    TitleListModel *titleListModel;
    MapWindow *mapWindow;
    DumpSettings *dumpSettings;
};

#endif // MAINWINDOW_H
