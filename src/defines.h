#ifndef DEFINES_H
#define DEFINES_H

#define EVOPEDIA_VERSION "0.4.1"
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
