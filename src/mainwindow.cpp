#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QAbstractButton>

#include "evopediaapplication.h"
#include "mapwindow.h"
#include "dumpsettings.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Evopedia),
    titleListModel(new TitleListModel(this))
{
    titleListModel->setTitleIterator(TitleIterator());

    ui->setupUi(this);
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    foreach (StorageBackend *b, evopedia->getBackends())
       ui->languageChooser->addItem(b->getLanguage());
    ui->listView->setModel(titleListModel);

    connect(evopedia, SIGNAL(backendsChanged(const QList<StorageBackend*>)), SLOT(backendsChanged(const QList<StorageBackend*>)));
    connect(evopedia->findChild<EvopediaWebServer *>("evopediaWebserver"),
            SIGNAL(mapViewRequested(qreal, qreal, uint)),
            SLOT(mapViewRequested(qreal,qreal,uint)));

    QActionGroup *network = new QActionGroup(this);
    network->addAction(ui->actionAuto);
    network->addAction(ui->actionAllow);
    network->addAction(ui->actionDeny);

    QSettings settings("Evopedia", "GUI");
    int networkUse = settings.value("network use", 0).toInt();
    //evopedia->setNetworkUse(networkUse);
    if (networkUse < 0) ui->actionDeny->setChecked(true);
    else if (networkUse > 0) ui->actionAllow->setChecked(true);
    else ui->actionAuto->setChecked(true);

    QString defaultLanguage = settings.value("default language", "").toString();
    if (evopedia->hasLanguage(defaultLanguage)) {
        for (int i = 0; i < ui->languageChooser->count(); i ++) {
            if (ui->languageChooser->itemText(i) == defaultLanguage) {
                ui->languageChooser->setCurrentIndex(i);
                break;
            }
        }
    }

    QPointF mapPos = settings.value("map pos", QPointF(10.7387413, 59.9138204)).toPointF();
    int mapZoom = settings.value("map zoom", 15).toInt();

    /* TODO1 this should be improved:
       any key press that is accepted by
       the searchField should go to the searchField */
    /* TODO does not work
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(ui->searchField);
    setFocus();
    */
    ui->searchField->setFocus();

    mapWindow = new MapWindow(this);
    mapWindow->setPosition(mapPos.y(), mapPos.x(), mapZoom);
#ifndef Q_OS_SYMBIAN
    mapWindow->resize(600, 450);
#endif
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    mapWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
#endif

    if (evopedia->getBackends().length() == 0) {
        QMessageBox msgBox(QMessageBox::NoIcon, tr("No Dumps Configured"),
                           tr("To be able to use evopedia you have to "
                                   "download and install a Wikipedia dump. "
                                   "Download at least one dump file from the "
                                   "<a href=\"%1\">website</a> and extract "
                                   "this archive to a folder on your device. "
                                   "After that, select this folder using "
                                   "the menu option \"Configure Dumps\".")
                           .arg(EVOPEDIA_DUMP_SITE),
                           QMessageBox::Ok,
                           this);
        msgBox.exec();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("Evopedia", "GUI");
    int networkUse = 0;
    if (ui->actionDeny->isChecked()) networkUse = -1;
    else if (ui->actionAllow->isChecked()) networkUse = 1;
    settings.setValue("network use", networkUse);
    settings.setValue("default language", ui->languageChooser->currentText());

    qreal lat, lng;
    int zoom;
    mapWindow->getPosition(lat, lng, zoom);
    settings.setValue("map pos", QPointF(lng, lat));
    settings.setValue("map zoom", zoom);

    event->accept();
}

void MainWindow::on_searchField_textChanged(const QString &text)
{
    Q_UNUSED(text);
    refreshSearchResults();
}

void MainWindow::on_listView_activated(QModelIndex index)
{
    const Title title(titleListModel->getTitleAt(index));

    (static_cast<EvopediaApplication *>(qApp))->openArticle(title);
}

void MainWindow::on_languageChooser_currentIndexChanged(const QString &text)
{
    Qt::LayoutDirection dir = getLayoutDirection(text);
    ui->listView->setLayoutDirection(dir);
    ui->searchField->setLayoutDirection(dir);
    refreshSearchResults();
}

void MainWindow::backendsChanged(const QList<StorageBackend *> backends)
{
    ui->languageChooser->blockSignals(true);
    ui->languageChooser->clear();
    foreach (StorageBackend *b, backends)
       ui->languageChooser->addItem(b->getLanguage());
    ui->languageChooser->blockSignals(false);
    refreshSearchResults();
}

void MainWindow::mapViewRequested(qreal lat, qreal lon, uint zoom)
{
    mapWindow->setPosition(lat, lon, zoom);
    showMapWindow();
}

void MainWindow::refreshSearchResults()
{
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    StorageBackend *backend = evopedia->getBackend(ui->languageChooser->currentText());
    TitleIterator it;
    if (backend != 0)
        it = backend->getTitlesWithPrefix(ui->searchField->text());
    titleListModel->setTitleIterator(it);
}

void MainWindow::on_actionMap_triggered()
{
    /* TODO0 show current location? */
    showMapWindow();
}

void MainWindow::showMapWindow()
{
#if defined(Q_OS_SYMBIAN)
    mapWindow->showMaximized();
#else
    mapWindow->show();
#endif
    mapWindow->raise();
    mapWindow->activateWindow();
}

void MainWindow::on_actionConfigure_Dumps_triggered()
{
    /* TODO2 list dump files and download them automatically */

    DumpSettings *dumpSettings = new DumpSettings(this);
    dumpSettings->show();
    /* TODO wait until closed? only one instance? */
}

void MainWindow::on_actionAbout_triggered()
{
    const QString version(EVOPEDIA_VERSION);
    QMessageBox msgBox(QMessageBox::NoIcon, tr("About Evopedia"),
                             tr("<h2>Evopedia %1</h2>"
                             "Offline Wikipedia Viewer"
                             "<p><b>Copyright Information<b><br/>"
                             "<small>This program shows articles from "
                             "<a href=\"http://wikipedia.org\">Wikipedia</a>, "
                             "available under the "
                             "<a href=\"http://creativecommons.org/licenses/by-sa/3.0/\">"
                             "Creative Commons Attribution/Share-Alike License</a>. "
                             "Further information can be found via the links "
                             "to the online versions of the respective "
                             "articles.</small></p>").arg(version), QMessageBox::Ok, this);
    msgBox.setIconPixmap(QPixmap(":/static/wikipedia.png"));
    QPushButton *websiteButton = msgBox.addButton("Visit Website", QMessageBox::AcceptRole);
    QPushButton *downloadButton = msgBox.addButton("Download Dumps", QMessageBox::AcceptRole);
    QPushButton *bugButton = msgBox.addButton("Report Bug", QMessageBox::AcceptRole);

    msgBox.exec();

    QPushButton *clickedButton = dynamic_cast<QPushButton*>(msgBox.clickedButton());

    if (clickedButton == websiteButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_WEBSITE));
    } else if (clickedButton == downloadButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_DUMP_SITE));
    } else if (clickedButton == bugButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_BUG_SITE));
    }
}

void MainWindow::on_actionAuto_toggled(bool v)
{
    if (v) {
        Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
        evopedia->setNetworkUse(0);
    }
}

void MainWindow::on_actionAllow_toggled(bool v)
{
    if (v) {
        Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
        evopedia->setNetworkUse(1);
    }
}

void MainWindow::on_actionDeny_toggled(bool v)
{
    if (v) {
        Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
        evopedia->setNetworkUse(-1);
    }
}
