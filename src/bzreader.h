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

#ifndef BZREADER_H
#define BZREADER_H

#include <QByteArray>
#include <QFile>

#include <bzlib.h>

class BZReader
{
public:
    BZReader();
    const QByteArray readAt(QFile &f, quint32 blockStart, quint32 blockOffset, quint32 dataLength);
private:
    bz_stream stream;
};

#endif // BZREADER_H
