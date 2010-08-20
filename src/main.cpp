#include <QtGui/QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include "mainwindow.h"
#include "evopediaapplication.h"

#include "map.h"

int main(int argc, char *argv[])
{
    EvopediaApplication app(argc, argv);
    return app.exec();
}
