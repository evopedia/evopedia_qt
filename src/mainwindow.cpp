#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

#include "mapwindow.h"
#include "dumpsettings.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Evopedia),
    titleListModel(new TitleListModel(this))
{
    evopedia = new Evopedia(this);
    evopedia->setObjectName("evopedia");
    titleListModel->setTitleIterator(TitleIterator());

    ui->setupUi(this);
    foreach (StorageBackend *b, evopedia->getBackends())
       ui->languageChooser->addItem(b->getLanguage());
    ui->listView->setModel(titleListModel);

    /* TODO1 this should be improved:
       any key press that is accepted by
       the searchField should go to the searchField */
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(ui->searchField);
    setFocus();

    mapWindow = new MapWindow(evopedia, this);
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

void MainWindow::on_searchField_textChanged(const QString &text)
{
    refreshSearchResults();
}

void MainWindow::on_listView_activated(QModelIndex index)
{
    const Title title(titleListModel->getTitleAt(index));

    QDesktopServices::openUrl(evopedia->getArticleUrl(title));
}

void MainWindow::on_languageChooser_currentIndexChanged(const QString &text)
{
    Qt::LayoutDirection dir = getLayoutDirection(text);
    ui->listView->setLayoutDirection(dir);
    ui->searchField->setLayoutDirection(dir);
    refreshSearchResults();
}

void MainWindow::on_evopedia_backendsChanged(const QList<StorageBackend *> backends)
{
    ui->languageChooser->blockSignals(true);
    ui->languageChooser->clear();
    foreach (StorageBackend *b, backends)
       ui->languageChooser->addItem(b->getLanguage());
    ui->languageChooser->blockSignals(false);
    refreshSearchResults();
}

void MainWindow::on_evopediaWebserver_mapViewRequested(qreal lat, qreal lon, uint zoom)
{
    mapWindow->setPosition(lat, lon, zoom);
    showMapWindow();
}

void MainWindow::refreshSearchResults()
{
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

    DumpSettings dumpSettings(evopedia, this);
    dumpSettings.exec();
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

    if (msgBox.clickedButton() == websiteButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_WEBSITE));
    } else if (msgBox.clickedButton() == downloadButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_DUMP_SITE));
    } else if (msgBox.clickedButton() == bugButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_BUG_SITE));
    }
}
