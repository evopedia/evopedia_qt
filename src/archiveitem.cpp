#include "archiveitem.h"

ArchiveItem::ArchiveItem() : QStandardItem() {
// 3 types of backends
//  - localArchive
//  - remoteArchive
//  - torrentArchive (either complete or incomplete)
    extend();
}

void ArchiveItem::changeBackend(int type) {
    switch(type) {
    case 1: // localArchive
        // new blah
        break;
    case 2: // remoteArchive
        // new blah
        break;
    case 3: // torrentArchive (either complete or incomplete)
        break;
    }
}

void ArchiveItem::extend() {
    QStandardItem *item1_1Col1 = new QStandardItem();                       // Spalte 1 vom ChildChild
    QStandardItem *item1_1Col2 = new QStandardItem(QString("item1_1Col2")); // Spalte 2 vom ChildChild
    QStandardItem *item1_1Col3 = new QStandardItem(QString("item1_1Col3")); // Spalte 3 vom ChildChild

    // Spalten vorbereiten
    QList<QStandardItem*> itemList;
    itemList.append(item1_1Col1);
    itemList.append(item1_1Col2);
    itemList.append(item1_1Col3);

    appendRow(itemList); // Child an Child hngen
}


