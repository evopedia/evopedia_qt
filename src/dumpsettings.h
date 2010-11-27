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
