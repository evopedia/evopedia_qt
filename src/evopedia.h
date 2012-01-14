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

#ifndef EVOPEDIA_H
#define EVOPEDIA_H

#include "evopediawebserver.h"
#include "archivemanager.h"

class Evopedia;
class EvopediaWebServer;
class Title;

class Evopedia : public QObject
{
    Q_OBJECT
public:
    explicit Evopedia(QObject *parent=0, bool guiEnabled=true);
    QUrl getArticleUrl(const Title &t) const;
    void setNetworkUse(int use);
    bool networkConnectionAllowed();
    bool isGUIEnabled() const { return guiEnabled; }
    ArchiveManager *getArchiveManager() const { return archiveManager; }
private:
    ArchiveManager *archiveManager;
    EvopediaWebServer *webServer;
    int networkUse;
    bool guiEnabled;
};

#endif // EVOPEDIA_H
