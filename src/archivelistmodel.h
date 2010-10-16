#ifndef ARCHIVELISTMODEL_H
#define ARCHIVELISTMODEL_H

#include <QTreeWidget>
#include <QHash>
#include <QPair>
#include <QString>

#include "archiveitem.h"

class ArchiveListModel : public QObject
{
    Q_OBJECT

    QTreeWidget *widget;

    /* (language, date) -> archive item */
    QHash<QPair<QString, QString>, ArchiveListItem *> archiveItems;



public:
    explicit ArchiveListModel(QTreeWidget *widget, QObject *parent = 0);

    void updateDownloadableArchives(const QString &data);

signals:

public slots:

};

#endif // ARCHIVELISTMODEL_H
