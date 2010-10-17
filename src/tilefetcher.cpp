#include "tilefetcher.h"

#include <QMutexLocker>

#include "defines.h"
#include "evopedia.h"
#include "evopediaapplication.h"

TileFetcher::TileFetcher(QObject *parent) :
    QThread(parent)
{
}

void TileFetcher::run()
{
    netManager = new QNetworkAccessManager(this);
    connect(netManager, SIGNAL(finished(QNetworkReply*)), SLOT(networkRequestCompleted(QNetworkReply*)));

    exec();
}

void TileFetcher::fetchTile(int zoom, QPoint offset)
{
    Q_ASSERT(0 <= offset.x() && offset.x() < (1 << zoom));
    Q_ASSERT(0 <= offset.y() && offset.y() < (1 << zoom));

    /* fetch from local cache */
    QImage img(QString("%1/%2/%3/%4/%5.png")
                        .arg(MAPTILES_LOCATION)
                        .arg("OpenStreetMap I")
                        .arg(zoom)
                        .arg(offset.x())
                        .arg(offset.y()), "PNG");
    if (!img.isNull()) {
        emit tileLoaded(zoom, offset, img);
        return;
    }

    /* fetch from server */
    QString path = "http://tile.openstreetmap.org/%1/%2/%3.png";
    QUrl url(path.arg(zoom).arg(offset.x()).arg(offset.y()));

    /* TODO not thread safe! */
    Evopedia *evopedia = (static_cast<EvopediaApplication *>(qApp))->evopedia();
    if (!evopedia->networkConnectionAllowed() /* TODO && !m_manager.cache()->metaData(m_url).isValid()*/)
        return;

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "Nokia (Qt) Graphics Dojo 1.0");
    request.setAttribute(QNetworkRequest::User, QVariant(zoom));
    request.setAttribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1), QVariant(offset));
    netManager->get(request);
}

void TileFetcher::networkRequestCompleted(QNetworkReply *reply)
{
    QImage img;
    int zoom = reply->request().attribute(QNetworkRequest::User).toInt();
    QPoint offset = reply->request().attribute(QNetworkRequest::Attribute(QNetworkRequest::User + 1)).toPoint();

    reply->deleteLater();
    if (reply->error() || !img.load(reply, "PNG"))
        return;

    /* TODO1 on maemo only if MyDocs is mounted! */
    QString path = QString("%1/%2/%3/%4")
                   .arg(MAPTILES_LOCATION)
                   .arg("OpenStreetMap I")
                   .arg(zoom)
                   .arg(offset.x());
    QDir().mkpath(path);
    img.save(QString("%1/%2.png").arg(path).arg(offset.y()));

    emit tileLoaded(zoom, offset, img);
}
