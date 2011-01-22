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

#include "title.h"

#include <QDataStream>

Title::Title()
    : fileNr(0), blockStart(0), blockOffset(0), articleLength(0)
{
}

Title::Title(const QByteArray &encodedTitle, const QString &language)
    : language(language), fileNr(255)
{
    if (encodedTitle.length() < 15)
        return;

    QByteArray escapeData(encodedTitle.left(2));
    QByteArray positionData(encodedTitle.mid(2, 13));

    QDataStream escapeDataStream(escapeData);
    escapeDataStream.setByteOrder(QDataStream::LittleEndian);

    quint16 escapes;
    escapeDataStream >> escapes;
    
    for (int i = 0; i < 13; i ++)
        if (escapes & (1 << i))
            positionData[i] = '\n';
    
    QDataStream positionDataStream(positionData);
    positionDataStream.setByteOrder(QDataStream::LittleEndian);
    
    positionDataStream >> fileNr >> blockStart >> blockOffset >> articleLength;
    
    int titleLenBytes = encodedTitle.length() - 15;
    if (titleLenBytes > 0 && encodedTitle[encodedTitle.length() - 1] == '\n')
        titleLenBytes --;
    
    name = QString::fromUtf8(encodedTitle.mid(15, titleLenBytes).constData(),
                            titleLenBytes);
}
