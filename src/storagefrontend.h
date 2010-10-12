#ifndef STORAGEFRONTEND_H
#define STORAGEFRONTEND_H

#include "abstractfrontend.h"
#include "storagebackend.h"

class QMenu;
class QString;
class ArchiveItem;

class StorageFrontend : public AbstractFrontend {
    Q_OBJECT
    friend class ArchiveItem;

protected:
    StorageFrontend(ArchiveItem* item, QString archiveDir);
    ~StorageFrontend();
    QMenu* createContextMenu();
    void saveSettings();
    void unsaveSettings();
    bool validate(QString &ret);
    StorageBackend* storageBackend();
    bool m_activated;
public:
    QString language();
    QString date();
    QString archiveDir();
    QString size();
    QString stateString();
private:
    ArchiveItem* m_archiveitem;
    StorageBackend *m_storageBackend;
    QString m_language;
    QString m_date;
    QString m_size;
    QString m_archiveDir;
    QString m_stateString;  // message for the user
Q_SIGNALS:
    void updateBackends();
public Q_SLOTS:
    void removeEntry();
};

#endif // STORAGEFRONTEND_H
