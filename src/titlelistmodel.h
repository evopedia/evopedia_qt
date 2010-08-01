#ifndef TITLELISTMODEL_H
#define TITLELISTMODEL_H

#include <QAbstractListModel>
#include <QFile>

#include "title.h"
#include "titleiterator.h"

class TitleListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    TitleListModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    const Title getTitleAt(const QModelIndex &index) const;

protected:
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

signals:
    void numberPopulated(int number);

public slots:
    void setTitleIterator(TitleIterator iter);

private:
    TitleIterator titleIter;
    QList<Title> titles;

};

#endif // TITLELISTMODEL_H
