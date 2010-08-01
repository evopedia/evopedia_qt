#include <QtGui/QApplication>
#include "mainwindow.h"

#include "map.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if defined(Q_WS_X11)
    QApplication::setGraphicsSystem("raster");
#endif

    MainWindow w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
