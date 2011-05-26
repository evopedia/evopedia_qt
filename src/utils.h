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

#ifndef UTILS_H
#define UTILS_H

#include <QtGlobal>
#include <QPair>
#include <QByteArray>

quint32 randomNumber(quint32 maxExcl);
QPair<qreal, qreal> parseCoordinatesInArticle(QByteArray &text, bool *error=0, int *zoom=0);
int parseCoordinatesZoom(const QString &zoomstr);
inline Qt::LayoutDirection getLayoutDirection(const QString &language)
{
    /* TODO1 are these all RTL languages of Wikipedia? */
    if (language == "ar" || language == "he" || language == "yi" || language == "ur" || language == "ckb") {
        return Qt::RightToLeft;
    } else {
        return Qt::LeftToRight;
    }
}

#endif // UTILS_H
