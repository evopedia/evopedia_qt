#include "mapwindow.h"
#include "ui_mapwindow.h"

#ifdef Q_WS_MAEMO_5
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif


#include <QTimer>

MapWindow::MapWindow(Evopedia *evopedia, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MapWindow)
{
    ui->setupUi(this);
    ui->mapWidget->setEvopedia(evopedia);
    ui->mapWidget->setFocus();

    connect(ui->actionShow_Articles, SIGNAL(toggled(bool)), ui->mapWidget, SLOT(overlaysEnable(bool)));
    connect(ui->actionZoom_In, SIGNAL(triggered()), ui->mapWidget, SLOT(zoomIn()));
    connect(ui->actionZoom_Out, SIGNAL(triggered()), ui->mapWidget, SLOT(zoomOut()));


    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

MapWindow::~MapWindow()
{
    delete ui;
}

#ifdef Q_WS_MAEMO_5
void MapWindow::grabZoomKeys(bool grab)
{
    if (!winId()) {
        qWarning("Can't grab keys unless we have a window id");
        return;
    }

    unsigned long val = (grab) ? 1 : 0;
    Atom atom = XInternAtom(QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", False);
    if (!atom) {
        qWarning("Unable to obtain _HILDON_ZOOM_KEY_ATOM. This example will only work "
                 "on a Maemo 5 device!");
        return;
    }

    XChangeProperty (QX11Info::display(),
                     winId(),
                     atom,
                     XA_INTEGER,
                     32,
                     PropModeReplace,
                     reinterpret_cast<unsigned char *>(&val),
                     1);
}
#endif


void MapWindow::setPosition(qreal lat, qreal lon, int zoom)
{
    ui->mapWidget->setPosition(lat, lon, zoom);
}
