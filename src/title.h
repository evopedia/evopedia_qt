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

#ifndef TITLE_H
#define TITLE_H

#include <QString>

class Title
{
public:
    Title();
    Title(const QByteArray &encodedTitle, const QString &language);
    const QString &getName() const { return name; }
    const QString &getLanguage() const { return language; }
    const QString getReadableName() const { return QString(name).replace('_', ' '); }
    quint8 getFileNr() const { return fileNr; }
    quint32 getBlockStart() const { return blockStart; }
    quint32 getBlockOffset() const { return blockOffset; }
    quint32 getArticleLength() const { return articleLength; }
private:
    QString name;
    QString language;
    quint8 fileNr;
    quint32 blockStart;
    quint32 blockOffset;
    quint32 articleLength;
};

#endif // TITLE_H
