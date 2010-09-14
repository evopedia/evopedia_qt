#ifndef ARCHIVEITEM_H
#define ARCHIVEITEM_H

#include <QStandardItem>

class ArchiveItem : protected QStandardItem {
  friend class ArchiveManager;

public:
    ArchiveItem();
    void extend();
    void changeBackend(int type);
protected:
    //void setData ( const QVariant & value, int role = Qt::UserRole + 1 );
};

#endif // ARCHIVEITEM_H
