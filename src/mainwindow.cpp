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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QPushButton>
#include <QAbstractButton>

#include "evopediaapplication.h"
#include "mapwindow.h"
#include "dumpsettings.h"
#include "utils.h"
#include "defines.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Evopedia),
    titleListModel(new TitleListModel(this))
{
    titleListModel->setTitleIterator(TitleIterator());

    ui->setupUi(this);
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    foreach (LocalArchive *b, evopedia->getArchiveManager()->getDefaultLocalArchives())
       ui->languageChooser->addItem(b->getLanguage());
    ui->listView->setModel(titleListModel);

    connect(evopedia->getArchiveManager(),
            SIGNAL(defaultLocalArchivesChanged(QList<LocalArchive*>)),
            SLOT(backendsChanged(QList<LocalArchive*>)));
    connect(evopedia->findChild<EvopediaWebServer *>("evopediaWebserver"),
            SIGNAL(mapViewRequested(qreal, qreal, uint)),
            SLOT(mapViewRequested(qreal,qreal,uint)));

    QActionGroup *network = new QActionGroup(this);
    network->addAction(ui->actionAuto);
    network->addAction(ui->actionAllow);
    network->addAction(ui->actionDeny);

    QSettings settings("Evopedia", "GUI");
    int networkUse = settings.value("network use", 1).toInt();
    evopedia->setNetworkUse(networkUse);
    if (networkUse < 0) ui->actionDeny->setChecked(true);
    else if (networkUse > 0) ui->actionAllow->setChecked(true);
    else ui->actionAuto->setChecked(true);

    QString defaultLanguage = settings.value("default language", "").toString();
    if (evopedia->getArchiveManager()->hasLanguage(defaultLanguage)) {
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

    dumpSettings = new DumpSettings(this);
#ifndef Q_OS_SYMBIAN
    mapWindow->resize(600, 450);
#endif
#ifdef Q_WS_MAEMO_5
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    mapWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
    dumpSettings->setAttribute(Qt::WA_Maemo5StackedWindow);
#endif

    ArchiveManager *archiveManager = evopedia->getArchiveManager();

    if (archiveManager->getDefaultLocalArchives().isEmpty()) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
                              tr("No Archives Configured"),
                              tr("To be able to use evopedia you have to "
                                      "download a Wikipedia archive. "
                                      "This can be done from within evopedia "
                                      "via the menu option \"Archives\". "
                                      "If you only want to try out evopedia, "
                                      "you can use the language \"small\", which "
                                      "is a small version of the English Wikipedia.<br />"
                                      "Do you want to download an archive now?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes);
        if (answer == QMessageBox::Yes) {
            dumpSettings->show();
            archiveManager->updateRemoteArchives();
        }
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

void MainWindow::backendsChanged(QList<LocalArchive *> backends)
{
    ui->languageChooser->blockSignals(true);
    ui->languageChooser->clear();
    foreach (LocalArchive *b, backends)
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
    LocalArchive *backend = evopedia->getArchiveManager()->getLocalArchive(ui->languageChooser->currentText());
    TitleIterator it;
    if (backend != 0)
        it = backend->getTitlesWithPrefix(ui->searchField->text());
    titleListModel->setTitleIterator(it);
}

void MainWindow::on_actionMap_triggered()
{
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
    dumpSettings->show();

    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    if (evopedia->getArchiveManager()->getArchives().size() == 0) {
        QMessageBox::information(dumpSettings, tr("Archive Download"),
                                 tr("Use the menu to retrieve the list of available archives."));
        return;
    }
}

void MainWindow::on_actionAbout_triggered()
{
    const QString version(EVOPEDIA_VERSION);
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("About Evopedia"));
    msgBox.setText(tr("<h2>Evopedia %1</h2>"
                             "<p>Offline Wikipedia Viewer</p>"
                             "<p>Copyright Information<br/>"
                             "<small>This program shows articles from "
                             "<a href=\"http://wikipedia.org\">Wikipedia</a>, "
                             "available under the "
                             "<a href=\"http://creativecommons.org/licenses/by-sa/3.0/\">"
                             "Creative Commons Attribution/Share-Alike License</a>. "
                             "Further information can be found via the links "
                             "to the online versions of the respective "
                             "articles.</small></p>"
                             "<p>Authors<br/>"
                             "<small>"
                             "Code: Christian Reitwiessner, Joachim Schiele<br/>"
                             "Icon: Joachim Schiele<br/>"
                             "Translations: mossroy (French), Santiago Crespo (Spanish)"
                             "</small></p>").arg(version));
    msgBox.setIconPixmap(QPixmap(":/web/evopedia-64x64.png"));
    QPushButton *websiteButton = msgBox.addButton(tr("Visit Website"), QMessageBox::AcceptRole);
    QPushButton *translateButton = msgBox.addButton(tr("Translate"), QMessageBox::AcceptRole);
    QPushButton *bugButton = msgBox.addButton(tr("Report Bug"), QMessageBox::AcceptRole);
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);

    msgBox.exec();

    QPushButton *clickedButton = dynamic_cast<QPushButton*>(msgBox.clickedButton());

    if (clickedButton == websiteButton) {
        QDesktopServices::openUrl(QUrl(EVOPEDIA_WEBSITE));
    } else if (clickedButton == translateButton) {
        QMessageBox::information(this, tr("Translate Evopedia"),
                                 tr("To translate Evopedia to your language, download the translation file "
                                    "from the website (<a href=\"http://evopedia.info\">evopedia.info</a>) "
                                    "and send it back to devs@evopedia.info."),
                                 QMessageBox::Ok);
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
