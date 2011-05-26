/*
 * evopedia: An offline Wikipedia reader.
 *
 * Copyright (C) 2010-2011 evopedia developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QVector>

#include "titlelistmodel.h"


TitleListModel::TitleListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int TitleListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return titles.size();
}

bool TitleListModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
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
    Q_UNUSED(parent);
    QList<Title> newTitles;
    while (titleIter.hasNext() && newTitles.size() < 50) {
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
