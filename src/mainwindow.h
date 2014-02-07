/*
 * evopedia: An offline Wikipedia reader.
 *
 * Copyright (C) 2010-2011 evopedia developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
    void on_actionLocal_toggled(bool v);
    void on_actionNetwork_toggled(bool v);
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
