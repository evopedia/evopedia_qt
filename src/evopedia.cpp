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

#include <QNetworkInterface>
#include "evopedia.h"
#include "utils.h"
#include "localarchive.h"

Evopedia::Evopedia(QObject *parent, bool guiEnabled)
    : QObject(parent), networkUse(0), guiEnabled(guiEnabled)
{
    archiveManager = new ArchiveManager(this);
    webServer = new EvopediaWebServer(this);
    webServer->setObjectName("evopediaWebserver");
}

QUrl Evopedia::getArticleUrl(const Title &t) const
{
    /* TODO1 direct link to title (not via name); include date? */
    return QUrl(QString("http://127.0.0.1:%1/wiki/%2/%3")
                .arg(webServer->serverPort())
                .arg(t.getLanguage())
                .arg(t.getName()));
}

void Evopedia::setNetworkUse(int use)
{
    networkUse = use;
}

bool Evopedia::networkConnectionAllowed()
{
    if (networkUse == 0) {
        /* see if we are online */
        QNetworkInterface wlan = QNetworkInterface::interfaceFromName("wlan0");
        QNetworkInterface gprs = QNetworkInterface::interfaceFromName("gprs0");

        return (wlan.isValid() && wlan.flags().testFlag(QNetworkInterface::IsUp)) ||
               (gprs.isValid() && gprs.flags().testFlag(QNetworkInterface::IsUp));
    } else {
        return networkUse > 0;
    }
}
