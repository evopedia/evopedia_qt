#include <QMouseEvent>
#include <QMenu>
#include <QStandardItemModel>
#include <QDebug>

#include "treeview.h"
#include "archiveitem.h"

TreeView::TreeView(QWidget * parent = 0) : QTreeView(parent) {
}

void TreeView::mouseReleaseEvent ( QMouseEvent * event ) {
    //#ifdef Q_WS_MAEMO_5
    QPoint a = event->pos();
    QModelIndex clickedIndex = indexAt(a);
    if (clickedIndex.isValid()) {
        QModelIndex parentIndex = clickedIndex.parent();
        if (parentIndex.isValid()) {
            // if is valid, then someone clicked a archive -> popup the custom qmenu
            QModelIndex archiveItemIndex = parentIndex.child(0, 0);
            if (!archiveItemIndex.isValid()) {
                return;
            }
            const QStandardItemModel* model = static_cast<const QStandardItemModel*>(archiveItemIndex.model());
            QStandardItem* standardItem = model->itemFromIndex(archiveItemIndex);
            if (standardItem->type() == QStandardItem::UserType + 1) {
                // dangerous cast, everyone warned me of, still i'm using it! (js)
                ArchiveItem* archiveItem = dynamic_cast<ArchiveItem*>(standardItem);
                if (archiveItem) {
                   QMenu* m = archiveItem->createContextMenu();
                   m->exec(mapToGlobal(event->pos()));
                }
            }
        }
    }
    //#endif
    QTreeView::mouseReleaseEvent(event);
}

/*! will automatically expand the torrent items childs, like clicking the '+'-sign on the tree */
void TreeView::rowsInserted ( const QModelIndex & parent, int start, int end ) {
    QTreeView::rowsInserted(parent,start,end);
    if (parent.data(Qt::UserRole + 1).toBool())
        expand(parent);
}
