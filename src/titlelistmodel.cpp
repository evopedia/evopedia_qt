#include <QVector>

#include "titlelistmodel.h"


TitleListModel::TitleListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int TitleListModel::rowCount(const QModelIndex &parent) const
{
    return titles.size();
}

bool TitleListModel::canFetchMore(const QModelIndex &parent) const
{
    return titleIter.hasNext();
}

void TitleListModel::setTitleIterator(TitleIterator iter)
{
    titleIter = iter;
    titles = QList<Title>();

    reset();
}

void TitleListModel::fetchMore(const QModelIndex &parent)
{
    QList<Title> newTitles;
    while (titleIter.hasNext() && newTitles.size() < 30) {
        newTitles += titleIter.next();
    }
    beginInsertRows(QModelIndex(), titles.size(), titles.size() + newTitles.size());
    titles += newTitles;
    endInsertRows();

    emit numberPopulated(newTitles.size());
}

QVariant TitleListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= titles.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
        return titles[index.row()].getReadableName();

    return QVariant();
}

const Title TitleListModel::getTitleAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return Title();

    if (index.row() >= titles.size() || index.row() < 0)
        return Title();

    return titles[index.row()];
}
