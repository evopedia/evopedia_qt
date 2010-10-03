#include <QMouseEvent>
#include <QMenu>
#include <QStandardItemModel>

#include "treeview.h"
#include "archiveitem.h"

TreeView::TreeView(QWidget * parent = 0) : QTreeView(parent) {
}

void TreeView::mouseReleaseEvent ( QMouseEvent * event ) {
    QPoint a = event->pos();
    QModelIndex clickedIndex = indexAt(a);
    if (clickedIndex.isValid()) {
        QModelIndex parentIndex = clickedIndex.parent();
        if (parentIndex.isValid()) {
            // if is valid, then someone clicked a archive -> popup the custom qmenu
            QModelIndex archiveItemIndex = parentIndex.child(0, 0);
            const QStandardItemModel* model = static_cast<const QStandardItemModel*>(archiveItemIndex.model());
            QStandardItem* standardItem = model->itemFromIndex(archiveItemIndex);
            if (standardItem->type() == QStandardItem::UserType + 1) {
                ArchiveItem* archiveItem = dynamic_cast<ArchiveItem*>(standardItem);
                QMenu* m = archiveItem->createContextMenu();
                m->exec(mapToGlobal(event->pos()));
            }
        }
    }
    QTreeView::mouseReleaseEvent(event);
}

