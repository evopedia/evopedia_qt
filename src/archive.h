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

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QObject>
#include <QPair>
#include <QString>
#include <QSettings>

/* (language, date) uniquely identifies archive */
typedef QPair<QString, QString> ArchiveID;

class Archive : public QObject
{
    Q_OBJECT

protected:
    QString language;
    QString date;
public:
    explicit Archive(QObject *parent = 0);

    const QString &getLanguage() const { return language; }
    const QString &getDate() const { return date; }
    const ArchiveID getID() const { return ArchiveID(language, date); }

    virtual void saveToSettings(QSettings &settings) const { Q_UNUSED(settings); }

    /* newer archives are "less" */
    bool operator<(const Archive &other) const {
        if (getLanguage() == other.getLanguage())
            return getDate() > other.getDate();
        else
            return getLanguage() < other.getLanguage();
    }

    static bool comparePointers(const Archive *a, const Archive *b) {
        return (*a) < (*b);
    }

signals:

public slots:


};

#endif // ARCHIVE_H
