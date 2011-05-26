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

#ifndef DEFINES_H
#define DEFINES_H

#define EVOPEDIA_VERSION "0.4.3"
#define EVOPEDIA_WEBSITE "http://evopedia.info"
#define EVOPEDIA_DUMP_SITE "http://dumpathome.evopedia.info/dumps/finished"
#define EVOPEDIA_BUG_SITE "https://bugs.maemo.org/enter_bug.cgi?product=evopedia"

#ifdef Q_WS_MAEMO_5
#define MAPTILES_LOCATION "/home/user/MyDocs/.maps"
#else
#define MAPTILES_LOCATION (QDir::homePath() + "/.cache/maps")
#endif
/* TODO0 symbian */

#endif // DEFINES_H
