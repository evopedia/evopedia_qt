#include "archivelist.h"

#include <QPushButton>
#include <QLabel>
#include <QProgressBar>

ArchiveList::ArchiveList(QWidget *parent) :
    QTreeWidget(parent)
{
    setColumnCount(4);
    setHeaderLabels(QStringList() << tr("lang/date") << "" << "" << tr("status"));

    // TODO fix this font metrics issue, remove hardcoded stuff
    /*
    setColumnWidth(0, 200);//fm.width("lang/2010-07-07");
    setColumnWidth(1, 120);//fm.width(""));
    setColumnWidth(2, 100);//fm.width(""));
    setColumnWidth(3, 100);//fm.width(""));
    */
}

void ArchiveList::exchangeArchives(DownloadableArchive *from, PartialArchive *to)
{
    for (int i = 0; i < topLevelItemCount(); i ++) {
        QTreeWidgetItem *parent = topLevelItem(i);
        if (parent->text(0) == from->getLanguage()) {
            for (int j = 0; j < parent->childCount(); j ++) {
                QTreeWidgetItem *item = parent->child(j);
                if (item->text(0) == from->getDate()) {
                    /* TODO clear item */
                    fillPartialArchiveItem(to, item);
                    return;
                }
            }
        }
    }
}

void ArchiveList::updateArchives(const QList<Archive *> &archivesOrig)
{
    QList<Archive *> archives(archivesOrig);
    qSort(archives.begin(), archives.end(), Archive::comparePointers);

    clear();
    QTreeWidgetItem *topItem(0);
    QString lastLanguage;

    foreach (Archive *a, archives) {
        if (lastLanguage != a->getLanguage()) {
            topItem = new QTreeWidgetItem(this, QStringList() << a->getLanguage());
            addTopLevelItem(topItem);
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(topItem);
        item->setText(0, a->getDate());

        /* TODO better using real polymorphism? But we need to keep UI and non-UI apart */
        if (qobject_cast<DownloadableArchive *>(a)) {
            fillDownloadableArchiveItem(static_cast<DownloadableArchive *>(a), item);
        } else if (qobject_cast<PartialArchive *>(a)) {
            fillPartialArchiveItem(static_cast<PartialArchive *>(a), item);
        } else if (qobject_cast<LocalArchive *>(a)) {
            fillLocalArchiveItem(static_cast<LocalArchive *>(a), item);
        }
    }
}

void ArchiveList::fillDownloadableArchiveItem(DownloadableArchive *a, QTreeWidgetItem *item)
{
    QString size = a->getSize();
    QString sizeMB = size.left(size.length() - 6);
    item->setText(1, QString("%1 MB").arg(sizeMB));

    QPushButton *button = new QPushButton("Start download");
    connect(button, SIGNAL(clicked()), a, SLOT(startDownload()));
    item->setSizeHint(3, button->sizeHint());
    setItemWidget(item, 3, button);
}

void ArchiveList::fillPartialArchiveItem(PartialArchive *a, QTreeWidgetItem *item)
{
    QProgressBar *pbar = new QProgressBar();
    pbar->setMinimum(0);
    pbar->setMaximum(100);
    item->setSizeHint(3, pbar->sizeHint());
    setItemWidget(item, 3, pbar);
    connect(a, SIGNAL(progressUpdated(int)), pbar, SLOT(setValue(int)));

    QTreeWidgetItem *subItem = new QTreeWidgetItem(item);
    QLabel *peerInfo = new QLabel();
    subItem->setSizeHint(1, peerInfo->sizeHint());
    setItemWidget(subItem, 1, peerInfo);
    connect(a, SIGNAL(peerInfoUpdated(QString)), peerInfo, SLOT(setText(QString)));

    QLabel *speedText = new QLabel();
    subItem->setSizeHint(2, speedText->sizeHint());
    setItemWidget(subItem, 2, speedText);
    connect(a, SIGNAL(speedTextUpdated(QString)), speedText, SLOT(setText(QString)));

    /* TODO always? */
    item->setExpanded(true);
}

void ArchiveList::fillLocalArchiveItem(LocalArchive *a, QTreeWidgetItem *item)
{
    item->setText(1, QString(tr("%n article(s)", "", a->getNumArticles())));
    /* should always be in use */
    item->setText(3, a->isReadable() ? tr("in use") : tr("error"));
}
