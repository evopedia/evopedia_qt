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

#ifndef DUMPSETTINGS_H
#define DUMPSETTINGS_H

#include <QMainWindow>
#include <QList>

#include "localarchive.h"

class Evopedia;

namespace Ui {
    class DumpSettings;
}

class DumpSettings : public QMainWindow
{
    Q_OBJECT
public:
    explicit DumpSettings(QWidget *parent = 0);
    ~DumpSettings();

private slots:
    void on_actionManually_add_archive_triggered();
    void on_actionChange_Default_Archive_Dir_triggered();
    void on_actionCompact_Layout_toggled(bool value);

private:
    Ui::DumpSettings *ui;
    Evopedia *evopedia;
};

#endif // DUMPSETTINGS_H
