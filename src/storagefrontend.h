#ifndef STORAGEFRONTEND_H
#define STORAGEFRONTEND_H

#include "abstractfrontend.h"
#include "localarchive.h"

#include <QString>
class QMenu;
class ArchiveListItem;

class StorageFrontend : public AbstractFrontend {
    Q_OBJECT
    friend class ArchiveListItem;

protected:
    StorageFrontend(ArchiveListItem* item, QString archiveDir = QString());
    ~StorageFrontend();
    QMenu* createContextMenu();
    void saveSettings();
    void unsaveSettings();
    bool validate(QString &ret);
    LocalArchive* storageBackend();
    bool m_activated;
public:
    QString language();
    QString date();
    QString archiveDir();
    QString size();
    QString stateString();
    void setArchiveDirectory(QString archiveDir);
private:
    ArchiveListItem* m_archiveitem;
    LocalArchive *m_storageBackend;
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
