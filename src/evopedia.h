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
    explicit Evopedia(QObject *parent=0);
    QUrl getArticleUrl(const Title &t) const;
    void setNetworkUse(int use);
    bool networkConnectionAllowed();
    ArchiveManager *archivemanager;
private:
    EvopediaWebServer *webServer;    
    int networkUse;
};

#endif // EVOPEDIA_H
