#include "evopedia.h"

#include <QSettings>
#include <QStringList>
#include <QDir>
#include <QRectF>
#include <QMessageBox>
#include <QNetworkInterface>

#include "utils.h"

Evopedia::Evopedia(QObject *parent)
    : QObject(parent), networkUse(0)
{
    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (settings.contains("evopedia/data_directory")) {
        /* old format */
        QString data_dir(settings.value("evopedia/data_directory").toString());
        settings.remove("evopedia/data_directory");
        settings.setValue("dump_UNKNOWN/data_directory", data_dir);
        settings.sync();
    }

    foreach (QString group, settings.childGroups()) {
        if (!group.startsWith("dump_"))
            continue;
        QString data_dir(settings.value(group + "/data_directory").toString());
        StorageBackend *backend = new StorageBackend(data_dir, this);
        if (!backend->isReadable()) {
            delete backend;
        } else {
            storages[backend->getLanguage()] = backend;
        }
    }
    webServer = new EvopediaWebServer(this);
    webServer->setObjectName("evopediaWebserver");
}

StorageBackend *Evopedia::getBackend(const QString language) const
{
    if (storages.contains(language))
        return storages[language];
    else
        return 0;
}

const QList<StorageBackend *> Evopedia::getBackends() const
{
    return storages.values();

}

StorageBackend *Evopedia::getRandomBackend() const
{
    quint32 numArticles = 0;
    foreach (StorageBackend *b, storages) {
        numArticles += b->getNumArticles();
    }

    quint32 articleId = randomNumber(numArticles);
    foreach (StorageBackend *b, storages) {
        quint32 bArticles = b->getNumArticles();
        if (bArticles > articleId) {
            return b;
        } else {
            articleId -= bArticles;
        }
    }
    return 0;
}

void Evopedia::addBackend(StorageBackend *backend)
{
    if (backend == 0 || !backend->isReadable())
        return;

    backend->setParent(this);

    const QString language(backend->getLanguage());

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(0, tr("Error"), tr("Unable to store settings."));
        delete backend;
        return;
    }
    settings.setValue(QString("dump_%1/data_directory").arg(language),
                          backend->getDirectory());
    if (storages.contains(language)) {
        if (storages[language] != backend)
            delete storages[language];
    }
    storages[language] = backend;
    settings.sync();

    const QList<StorageBackend *>backends = getBackends();
    emit backendsChanged(backends);
}

void Evopedia::removeBackend(StorageBackend *backend)
{
    if (backend == 0) return;

    const QString language(backend->getLanguage());
    if (!storages.contains(language)) return;
    if (storages[language] != backend) return;

    QSettings settings(QDir::homePath() + "/.evopediarc", QSettings::IniFormat);
    if (!settings.isWritable()) {
        QMessageBox::critical(0, tr("Error"), tr("Error storing settings."));
        return;
    }
    settings.remove(QString("dump_%1").arg(language));
    storages.remove(language);
    delete backend;
    settings.sync();

    const QList<StorageBackend *>backends = getBackends();
    emit backendsChanged(backends);
}

QUrl Evopedia::getArticleUrl(const Title &t) const
{
    /* TODO1 direct link to title (not via name) */
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
