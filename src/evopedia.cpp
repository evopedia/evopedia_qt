#include <QNetworkInterface>
#include "evopedia.h"
#include "utils.h"
#include "storagebackend.h"

Evopedia::Evopedia(QObject *parent)
    : QObject(parent), networkUse(0)
{
    archivemanager = new ArchiveManager(this); // FIXME (js) add this so that delete archivemanager is not needed
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
