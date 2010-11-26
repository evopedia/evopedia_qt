#include "archivelist.h"

#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>

#include "archivemanager.h"
#include "evopediaapplication.h"
#include "archivedetailsdialog.h"

ArchiveList::ArchiveList(QWidget *parent) :
    QTreeWidget(parent)
{
#if defined(Q_OS_SYMBIAN)
    setCompactLayout(true);
#else
    setCompactLayout(false);
#endif
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(itemClickedHandler(QTreeWidgetItem*, int)));

    downloadPausedMapper = new QSignalMapper(this);
    downloadStartedMapper = new QSignalMapper(this);
    showDetailsMapper = new QSignalMapper(this);

    connect(downloadPausedMapper, SIGNAL(mapped(QWidget*)), SLOT(downloadPausedHandler(QWidget*)));
    connect(downloadStartedMapper, SIGNAL(mapped(QWidget*)), SLOT(downloadStartedHandler(QWidget*)));
    connect(showDetailsMapper, SIGNAL(mapped(QObject*)), SLOT(showDetails(QObject*)));
}

void ArchiveList::exchangeArchives(DownloadableArchive *from, PartialArchive *to)
{
    for (int i = 0; i < topLevelItemCount(); i ++) {
        QTreeWidgetItem *parent = topLevelItem(i);
        if (parent->text(0) == from->getLanguage()) {
            for (int j = 0; j < parent->childCount(); j ++) {
                QTreeWidgetItem *item = parent->child(j);
                if (item->text(0) == from->getDate()) {
                    fillPartialArchiveItem(to, item);
                    item->setExpanded(true);
                    return;
                }
            }
        }
    }
}

void ArchiveList::exchangeArchives(PartialArchive *from, LocalArchive *to)
{
    for (int i = 0; i < topLevelItemCount(); i ++) {
        QTreeWidgetItem *parent = topLevelItem(i);
        if (parent->text(0) == from->getLanguage()) {
            for (int j = 0; j < parent->childCount(); j ++) {
                QTreeWidgetItem *item = parent->child(j);
                if (item->text(0) == from->getDate()) {
                    fillLocalArchiveItem(to, item);
                    item->setExpanded(false);
                    return;
                }
            }
        }
    }
}

void ArchiveList::updateArchives(const QList<Archive *> &archivesOrig)
{
    QSet<QString> expandedLanguages;
    QSet<QString> knownLanguages;
    QSet<ArchiveID> expandedItems;

    for (int i = 0; i < topLevelItemCount(); i ++) {
        QTreeWidgetItem *langItem = topLevelItem(i);
        const QString &lang = langItem->text(0);
        if (langItem->isExpanded())
            expandedLanguages += lang;
        knownLanguages += lang;
        for (int j = 0; j < langItem->childCount(); j ++) {
            QTreeWidgetItem *dateItem = langItem->child(j);
            if (dateItem->isExpanded())
                expandedItems += ArchiveID(lang, dateItem->text(0));
        }
    }

    clear();

    QList<Archive *> archives(archivesOrig);
    qSort(archives.begin(), archives.end(), Archive::comparePointers);

    QTreeWidgetItem *topItem(0);
    QString lastLanguage;

    foreach (Archive *a, archives) {
        if (lastLanguage != a->getLanguage() || a->getLanguage().isEmpty()) {
            lastLanguage = a->getLanguage();
            topItem = new QTreeWidgetItem(this, QStringList() << lastLanguage);
            addTopLevelItem(topItem);
            if (expandedLanguages.contains(lastLanguage))
                topItem->setExpanded(true);
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(topItem);
        item->setText(0, a->getDate());

        if (expandedItems.contains(a->getID()))
            item->setExpanded(true);

        if (qobject_cast<DownloadableArchive *>(a)) {
            fillDownloadableArchiveItem(static_cast<DownloadableArchive *>(a), item);
        } else if (qobject_cast<PartialArchive *>(a)) {
            if (!knownLanguages.contains(a->getLanguage()))
                topItem->setExpanded(true);
            fillPartialArchiveItem(static_cast<PartialArchive *>(a), item);
        } else if (qobject_cast<LocalArchive *>(a)) {
            if (!knownLanguages.contains(a->getLanguage()))
                topItem->setExpanded(true);
            fillLocalArchiveItem(static_cast<LocalArchive *>(a), item);
        }
    }
}

void ArchiveList::fillDownloadableArchiveItem(DownloadableArchive *a, QTreeWidgetItem *item)
{
    QPushButton *button = new QPushButton;
    int buttonPos;
    if (compactLayout) {
        button->setText(tr("Details"));
        showDetailsMapper->setMapping(button, a);
        connect(button, SIGNAL(clicked()), showDetailsMapper, SLOT(map()));
        buttonPos = 1;
    } else {
        QString size = a->getSize();
        QString sizeMB = size.left(size.length() - 6);
        item->setText(1, tr("%1 MB").arg(sizeMB));
        button->setText(tr("Start download"));

        connect(button, SIGNAL(clicked()), a, SLOT(startDownload()));
        buttonPos = 3;
    }
    item->setSizeHint(buttonPos, button->sizeHint());
    setItemWidget(item, buttonPos, button);
    /* TODO1 use icons */
}


void ArchiveList::fillPartialArchiveItem(PartialArchive *a, QTreeWidgetItem *item)
{
    QTreeWidgetItem *subItem = new QTreeWidgetItem(item);

    QPushButton *pauseButton = new QPushButton();
    pauseButton->setText(a->isDownloading() ? tr("Pause") : tr("Continue"));
    connect(pauseButton, SIGNAL(clicked()), a, SLOT(togglePauseDownload()));
    downloadPausedMapper->setMapping(a, pauseButton);
    downloadStartedMapper->setMapping(a, pauseButton);
    connect(a, SIGNAL(downloadStarted()), downloadStartedMapper, SLOT(map()));
    connect(a, SIGNAL(downloadPaused()), downloadPausedMapper, SLOT(map()));

    int pbarColumn;

    if (compactLayout) {
        pbarColumn = 1;

        subItem->setSizeHint(1, pauseButton->sizeHint());
        setItemWidget(subItem, 1, pauseButton);

        QPushButton *detailsButton = new QPushButton(tr("Details"));
        showDetailsMapper->setMapping(detailsButton, a);
        connect(detailsButton, SIGNAL(clicked()), showDetailsMapper, SLOT(map()));
        subItem->setSizeHint(0, detailsButton->sizeHint());
        setItemWidget(subItem, 0, detailsButton);
    } else {
        pbarColumn = 2;

        item->setSizeHint(3, pauseButton->sizeHint());
        setItemWidget(item, 3, pauseButton);

        item->setText(1, a->getSizeMB());

        QLabel *peerInfo = new QLabel();
        subItem->setSizeHint(1, peerInfo->sizeHint());
        setItemWidget(subItem, 1, peerInfo);
        connect(a, SIGNAL(peerInfoUpdated(QString)), peerInfo, SLOT(setText(QString)));

        QLabel *speedText = new QLabel();
        subItem->setSizeHint(2, speedText->sizeHint());
        setItemWidget(subItem, 2, speedText);
        connect(a, SIGNAL(speedTextUpdated(QString)), speedText, SLOT(setText(QString)));

        QLabel *statusText = new QLabel();
        subItem->setSizeHint(3, statusText->sizeHint());
        setItemWidget(subItem, 3, statusText);
        connect(a, SIGNAL(statusTextUpdated(QString)), statusText, SLOT(setText(QString)));
    }

    QProgressBar *pbar = new QProgressBar();
    pbar->setMinimum(0);
    pbar->setMaximum(100);
    item->setSizeHint(pbarColumn, pbar->sizeHint());
    setItemWidget(item, pbarColumn, pbar);
    connect(a, SIGNAL(progressUpdated(int)), pbar, SLOT(setValue(int)));

    a->emitStatusEvents();
}

void ArchiveList::fillLocalArchiveItem(LocalArchive *a, QTreeWidgetItem *item)
{
    if (item->childCount() > 0) {
        delete item->takeChild(0);
    }

    item->setText(1, QString(tr("%n article(s)", "", a->getNumArticles())));

    if (!compactLayout) {
        removeItemWidget(item, 2);
        removeItemWidget(item, 3);

        item->setText(2, "");

        QString statusText = tr("in use");
        if (a->isReadable()) {
            ArchiveManager *am = (static_cast<EvopediaApplication *>(qApp))->evopedia()->getArchiveManager();
            if (!am->isDefaultForLanguage(a))
                statusText = tr("inactive");
        } else {
            statusText = tr("error");
        }
        item->setText(3, statusText);
    }
}

void ArchiveList::itemClickedHandler(QTreeWidgetItem *item, int column)
{
    item->setExpanded(!item->isExpanded());
}

void ArchiveList::downloadPausedHandler(QWidget *widget)
{
    QPushButton *button = static_cast<QPushButton *>(widget);
    button->setText(tr("Continue"));
    /* TODO1 use icons */
}

void ArchiveList::downloadStartedHandler(QWidget *widget)
{
    QPushButton *button = static_cast<QPushButton *>(widget);
    button->setText(tr("Pause"));
    /* TODO1 use icons */
}

void ArchiveList::showDetails(QObject *o)
{
    ArchiveDetailsDialog *d;

    DownloadableArchive *da = qobject_cast<DownloadableArchive *>(o);
    if (da != 0) {
        d = new ArchiveDetailsDialog(da, this);
        if (d->exec() == QDialog::Accepted)
            da->startDownload();
        return;
    }
    PartialArchive *pa = qobject_cast<PartialArchive *>(o);
    if (pa != 0) {
        d = new ArchiveDetailsDialog(pa, this);
        d->exec();
        return;
    }

}

void ArchiveList::setCompactLayout(bool value)
{
    compactLayout = value;
    if (compactLayout) {
        setColumnCount(2);
        setHeaderLabels(QStringList() << tr("Language, Date") << "");
    } else {
        setColumnCount(4);
        setHeaderLabels(QStringList() << tr("Language, Date") << tr("Size") << "" << tr("Status"));
    }
    // TODO1 fix this font metrics issue, remove hardcoded stuff
    // TODO1 perhaps resizeColumnToContents() is of use
    setColumnWidth(0, 180);//fm.width("lang/2010-07-07");
    setColumnWidth(1, 190);//fm.width(""));
    setColumnWidth(2, 140);//fm.width(""));
    setColumnWidth(3, 100);//fm.width(""));
}
