#ifndef DUMPSETTINGS_H
#define DUMPSETTINGS_H

#include <QMainWindow>
#include <QList>

#include "storagebackend.h"

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
    void addButtonClicked();
    void refreshButtonClicked();

private:
    Ui::DumpSettings *ui;
    Evopedia *evopedia;
};

#endif // DUMPSETTINGS_H
