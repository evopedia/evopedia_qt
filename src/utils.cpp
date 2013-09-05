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

#include "utils.h"

#include <cstdlib>
#include <math.h>
#include <time.h>

#include <QRegExp>
#include <QPair>
#include <QHash>
#include <QNetworkInterface>
#include <QTime>

quint32 randomNumber(quint32 maxExcl)
{
    if (maxExcl == 0) return 0;
    static bool seedInitialized(false);
    if (!seedInitialized) {
        qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
        seedInitialized = true;
    }
    if (maxExcl > RAND_MAX) {
        quint32 a = randomNumber(maxExcl >> 4);
        quint32 b = randomNumber(1 << 4);
        return ((a << 4) | b) % maxExcl;
    } else {
        return qrand() % maxExcl;
    }
}

QString jsonEncodeString(QString str)
{
    QString encoded = "\"";
    for (int i = 0; i < str.length(); i ++) {
        QChar c = str[i];
        if (!c.isPrint()) {
            encoded += QString("\\u%1").arg(uint(c.unicode()), 4, 16, QLatin1Char('0')).toUtf8();
        } else if (c == '\\') {
            encoded += "\\\\";
        } else if (c == '"') {
            encoded += '\\';
        } else {
            encoded += c;
        }
    }
    return encoded + "\"";
}

QPair<qreal, qreal> parseCoordinatesInArticle(QByteArray &text, bool *ok, int *zoom)
{
    QRegExp ex("params=(\\d*\\.?\\d*)_(\\d*)_?(\\d*\\.?\\d*)_?(N|S)"
               "_(\\d*\\.?\\d*)_(\\d*)_?(\\d*\\.?\\d*)_?(E|W)([^\"\\']*)");
    int idx = 0;
    while ((idx = text.indexOf("params=", idx + 1)) > 0) {
        QString data = QString::fromUtf8(text.mid(idx, 255).data());
        if (ex.indexIn(data) < 0)
            continue;
        qreal lat = 0;
        for (int i = 1; i < 4; i ++) {
            bool cok = false;
            qreal v = ex.cap(i).toDouble(&cok);
            if (!cok) continue;
            lat += v * (pow(60.0, -(i - 1)));
        }
        if (ex.cap(4) == "S")
            lat = -lat;

        qreal lng = 0;
        for (int i = 5; i < 8; i ++) {
            bool cok = false;
            qreal v = ex.cap(i).toDouble(&cok);
            if (!cok) continue;
            lng += v * (pow(60.0, -(i - 5)));
        }
        if (ex.cap(8) == "W")
            lng = -lng;

        if (zoom) {
            *zoom = parseCoordinatesZoom(ex.cap(9));
        }

        if (ok != 0)
            *ok = true;
        return QPair<qreal, qreal>(lat, lng);
    }
    if (ok != 0)
        *ok = false;
    return QPair<qreal, qreal>();
}


int parseCoordinatesZoom(const QString &zoomstr)
{
    static QHash<QString, int> geoScaleByType;
    if (geoScaleByType.empty()) {
        geoScaleByType["country"] =     10000000;
        geoScaleByType["satellite"] =   10000000;
        geoScaleByType["state"] =        3000000;
        geoScaleByType["adm1st"] =       1000000;
        geoScaleByType["adm2nd"] =        300000;
        geoScaleByType["default"] =       300000;
        geoScaleByType["adm3rd"] =        100000;
        geoScaleByType["city"] =          100000;
        geoScaleByType["mountain"] =      100000;
        geoScaleByType["isle"] =          100000;
        geoScaleByType["river"] =         100000;
        geoScaleByType["waterbody"] =     100000;
        geoScaleByType["event"] =          50000;
        geoScaleByType["forest"] =         50000;
        geoScaleByType["glacier"] =        50000;
        geoScaleByType["airport"] =        30000;
        geoScaleByType["edu"] =            10000;
        geoScaleByType["pass"] =           10000;
        geoScaleByType["landmark"] =       10000;
        geoScaleByType["railwaystation"] = 10000;
    }

    int zoom = 12;

    QRegExp ex("_(scale|dim|type):(\\d*)([a-z0-9]*)");

    if (zoomstr.indexOf(ex) < 0) {
        return zoom;
    }

    float scale;
    bool ok;
    if (ex.cap(1) == "scale") {
        scale = ex.cap(2).toFloat(&ok);
        if (!ok)
            return zoom;
    } else if (ex.cap(1) == "dim") {
        scale = 10 * ex.cap(2).toFloat(&ok);
        if (!ok)
            return zoom;
    } else {
        QString type = ex.cap(2) + ex.cap(3);
        if (!geoScaleByType.contains(type))
            return zoom;
        scale = geoScaleByType[type];
    }

    zoom = qRound(28.7253 - log(scale) / log(2.0));
    return qBound(2, zoom, 18);
}
