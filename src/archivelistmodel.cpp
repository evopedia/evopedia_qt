#include "archivelistmodel.h"

ArchiveListModel::ArchiveListModel(QTreeWidget *widget, QObject *parent) :
    QObject(parent),
    widget(widget)
{
    setColumnCount(4);
    setHeaderData(0, Qt::Horizontal, QString("lang/date"));
    setHeaderData(1, Qt::Horizontal, QString(""));
    setHeaderData(2, Qt::Horizontal, QString(""));
    setHeaderData(3, Qt::Horizontal, QString("status"));
}



/*! used for torrent based local archives which either have the *.torrent still around or not */
ArchiveListItem* ArchiveListModel::addPartialArchive(QString language, QString date, QString archiveDir, QString torrent, QUrl url, QString& ret) {
    QString workingDir, size;
    //FIXME handle size parameter
    ArchiveListItem* item = new ArchiveListItem(language, date, size, workingDir, archiveDir, torrent, url);
    item->setData(true, Qt::UserRole + 1);
    item->validate(ret);
    return addArchive(item);
}

ArchiveListItem* ArchiveListModel::addArchive(ArchiveListItem* item) {
    connect(item->m_storagefrontend, SIGNAL(updateBackends()), SLOT(updateBackends()));

    // 1. find the language group
    QStandardItem* langItem = NULL;
    for(int i=0; i < m_model->rowCount(); ++i) {
        if (m_model->item(i,0)->text() == item->language()) {
            langItem = m_model->item(i,0);
            break;
        }
    }
    // 2. if no langItem found, we create it
    if (langItem == NULL) {
        langItem = new QStandardItem(item->language());
        langItem->setEditable(false);
        QStandardItem *item1_1Col2 = new QStandardItem();  // column 2 (starting by 1)
        QStandardItem *item1_1Col3 = new QStandardItem();  // column 3 (starting by 1)
        QStandardItem *item1_1Col4 = new QStandardItem();  // column 4 (starting by 1)
        item1_1Col2->setEditable(false);
        item1_1Col3->setEditable(false);
        item1_1Col4->setEditable(false);
        QList<QStandardItem*> itemList;
        itemList.append(langItem);
        itemList.append(item1_1Col2);
        itemList.append(item1_1Col3);
        itemList.append(item1_1Col4);
        m_model->appendRow(itemList);
    }

    // 3. append it as child with the right type
    QStandardItem *item1_1Col2 = new QStandardItem();  // column 2 (starting by 1)
    QStandardItem *item1_1Col3 = new QStandardItem();  // column 3
    QStandardItem *item1_1Col4 = new QStandardItem();  // column 4 (starting by 1)
    item->setEditable(false);
    item->setText(item->date());
    item1_1Col2->setEditable(false);
    item1_1Col3->setEditable(false);
    item1_1Col4->setEditable(false);
    // Spalten vorbereiten
    QList<QStandardItem*> itemList;
    itemList.append(item);
    itemList.append(item1_1Col2);
    itemList.append(item1_1Col3);
    itemList.append(item1_1Col4);
    langItem->appendRow(itemList); // Child an Child haengen
    item->update(); // mendatory function to update the all items in one row using the model

    emit updateBackends();
    return item;
}


