#ifndef EVOPEDIA_H
#define EVOPEDIA_H

#include <QHash>
#include <QList>

#include "storagebackend.h"
class Evopedia;
class EvopediaWebServer;
#include "evopediawebserver.h"

class Evopedia : public QObject
{
    Q_OBJECT
public:
    explicit Evopedia(QObject *parent=0);

    StorageBackend *getBackend(const QString language, const QString date=QString()) const;
    bool hasLanguage(const QString language) const { return storages.contains(language); }
    const QList<StorageBackend *> getBackends() const;
    StorageBackend *getRandomBackend() const;

    void addBackend(StorageBackend *backend);
    void removeBackend(StorageBackend *backend);

    QUrl getArticleUrl(const Title &t) const;

    void setNetworkUse(int use);
    bool networkConnectionAllowed();

signals:
    void backendsChanged(const QList<StorageBackend *> backends);

private:
    QHash<QString, QList<StorageBackend *> > storages;
    EvopediaWebServer *webServer;
    int networkUse;
};

#endif // EVOPEDIA_H
