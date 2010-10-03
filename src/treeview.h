#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>
#include <QPoint>

class TreeView : public QTreeView {
    Q_OBJECT
public:
    TreeView(QWidget * parent );
    void mouseReleaseEvent ( QMouseEvent * event );
signals:
    void showCustomContextMenu(const QPoint &);
};

#endif // TREEVIEW_H
