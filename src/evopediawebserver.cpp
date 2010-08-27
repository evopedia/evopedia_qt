#include "evopediawebserver.h"

#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QNetworkInterface>
#include <QRegExp>
#include <QLocale>
#include <QCryptographicHash>

#include "utils.h"

EvopediaWebServer::EvopediaWebServer(Evopedia *evopedia) :
    QTcpServer(evopedia), evopedia(evopedia)
{
    if (!listen(QHostAddress::LocalHost, 8080))
        listen(QHostAddress::LocalHost);
}

void EvopediaWebServer::incomingConnection(int socket)
{
    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(socket);
}

void EvopediaWebServer::readClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (!socket->canReadLine()) return;
    /* TODO1 wait for end of request header? peek? */

    const QList<QByteArray> tokens = socket->readLine().split(' ');
    if (tokens[0] != "GET" || tokens.size() < 2) {
        outputHeader(socket, "404");
        closeConnection(socket);
        return;
    }

    const QUrl url = QUrl::fromPercentEncoding(tokens[1]);

    QString path = url.path();
    if (path.endsWith("skins/common/images/magnify-clip.png"))
        path = "/static/magnify-clip.png";

    const QStringList pathParts = path.mid(1).split('/');
    if (pathParts.length() < 1) {
        outputIndexPage(socket);
        closeConnection(socket);
        return;
    }
    const QString firstPart = pathParts[0];
    if (firstPart == "static") {
        outputStatic(socket, pathParts);
    } else if (firstPart == "map") {
        qreal lat = url.queryItemValue("lat").toDouble();
        qreal lon = url.queryItemValue("lon").toDouble();
        int zoom = url.queryItemValue("zoom").toInt();
        mapViewRequested(lat, lon, zoom);
    } else if (firstPart == "random") {
        redirectRandom(socket, pathParts);
    } else if (firstPart == "math" ||
               (firstPart == "wiki" && pathParts.length() >= 2 && pathParts[1] == "math")) {
        outputMathImage(socket, pathParts);
    } else if (firstPart == "wiki" || firstPart == "articles") {
        outputWikiPage(socket, pathParts);
    } else {
        outputHeader(socket, "404");
    }
    closeConnection(socket);
}

void EvopediaWebServer::outputIndexPage(QTcpSocket *socket)
{
    /* TODO1 */
}

void EvopediaWebServer::outputHeader(QTcpSocket *socket, const QString responseCode, const QString contentType)
{
    /* TODO 404 should not have "Ok" */
    QString text = QString("HTTP/1.0 %1 Ok\r\n"
            "Content-Type: %2\r\n\r\n").arg(responseCode).arg(contentType);
    socket->write(text.toAscii());
}

void EvopediaWebServer::outputResponse(QTcpSocket *socket, const QByteArray &data, const QString contentType, bool cache)
{
    QString header = QString("HTTP/1.1 200 Ok\r\n"
            "Content-Type: %1\r\n"
            "Content-Length: %2\r\n")
            .arg(contentType).arg(data.length());
    if (cache) {
        QDateTime current(QDateTime::currentDateTime().toUTC());
        QString currentUTCDate(QLocale::c().toString(current,
                                                     "ddd, dd MMM yyyy hh:mm:ss"));
        QString modificationDate(QLocale::c().toString(current.addDays(-30),
                                                     "ddd, dd MMM yyyy hh:mm:ss"));
        header += QString("Date: %1 GMT\r\nLast-Modified: %2 GMT\r\n")
                  .arg(currentUTCDate)
                  .arg(modificationDate);
    }
    header += "\r\n";

    socket->write(header.toAscii() + data);
}

void EvopediaWebServer::outputRedirect(QTcpSocket *socket, const QUrl &url)
{
    QByteArray location;
    if (url.host().isEmpty()) {
        location = url.encodedPath();
    } else {
        location = url.toEncoded();
    }
    socket->write(QByteArray("HTTP/1.0 302 Ok\r\n"
            "Location: ") + location + QByteArray("\r\n\r\n"));
}

void EvopediaWebServer::outputStatic(QTcpSocket *socket, const QStringList &pathParts)
{
    if (pathParts.length() < 2) {
        outputHeader(socket, "404");
        return;
    }
    if (pathParts[1] == "main.css") {
        outputResponse(socket, getResource(":/web/main.css"), "text/css");
    } else {
        outputResponse(socket, getResource(":/web/" + pathParts[1]), "image/png");
    }
}

void EvopediaWebServer::redirectRandom(QTcpSocket *socket, const QStringList &pathParts)
{
    StorageBackend *backend = 0;
    if (pathParts.length() >= 2) {
        backend = evopedia->getBackend(pathParts[1]);
    } else {
        backend = evopedia->getRandomBackend();
    }
    if (backend == 0) {
        outputHeader(socket, "404");
        return;
    }
    Title t = backend->getRandomTitle();
    if (t.getName().isNull()) {
        outputHeader(socket, "404");
    } else {
        QUrl redirectTo(QString("/wiki/%1/%2").arg(backend->getLanguage()).arg(t.getName()));
        outputRedirect(socket, redirectTo);
    }
}

void EvopediaWebServer::outputMathImage(QTcpSocket *socket, const QStringList &pathParts)
{
    const QByteArray hexHash = QByteArray::fromHex(pathParts.last().left(32).toAscii());
    QByteArray data;
    foreach (StorageBackend *backend, evopedia->getBackends()) {
        data = backend->getMathImage(hexHash);
        if (!data.isNull())
            break;
    }
    if (data.isNull()) {
        outputHeader(socket, "404");
    } else {
        outputResponse(socket, data, "image/png");
    }
}

void EvopediaWebServer::outputWikiPage(QTcpSocket *socket, const QStringList &pathParts)
{
    if (pathParts.length() < 2) {
        outputHeader(socket, "404");
        return;
    }
    StorageBackend *backend = 0;
    if (pathParts.length() >= 3)
        backend = evopedia->getBackend(pathParts[1]);
    if (backend == 0) {
        if (pathParts[1].length() > 1 && evopedia->networkConnectionAllowed() && pathParts.length() >= 3) {
            /* TODO special characters in title */
            const QUrl redirectTo(QString("http://%1.wikipedia.org/wiki/%2").arg(pathParts[1]).arg(pathParts[2]));
            outputRedirect(socket, redirectTo);
            return;
        }
        foreach (StorageBackend *b, evopedia->getBackends()) {
            const Title t = b->getTitleFromPath(pathParts);
            if (t.getName().isEmpty())
                continue;
            QUrl redirectTo(QString("/wiki/%1/%2").arg(b->getLanguage()).arg(t.getName()));
            outputRedirect(socket, redirectTo);
            return;
        }
        outputHeader(socket, "404");
    } else {
        const Title t = backend->getTitleFromPath(pathParts);
        QByteArray articleData = backend->getArticle(t);
        if (articleData.isNull()) {
            QString lastPart = pathParts[pathParts.length() - 1];
            if (lastPart.contains(':') && (lastPart.endsWith(".png", Qt::CaseInsensitive) ||
                                           lastPart.endsWith(".svg", Qt::CaseInsensitive) ||
                                           lastPart.endsWith(".jpg", Qt::CaseInsensitive))
                                       && evopedia->networkConnectionAllowed()) {
                /* this could be a link to a file description page */
                outputRedirect(socket, QUrl(backend->getOrigUrl() + lastPart));
                return;
            } else {
               outputHeader(socket, "404");
               return;
            }
        }

        QByteArray data = getResource(":/web/header.html");

        bool ok(false);
        int zoom;
        QPair<qreal, qreal>coords = parseCoordinatesInArticle(articleData, &ok, &zoom);
        if (ok)
            data += QString("<a class=\"evopedianav\" "
                            "href=\"#\" onclick=\"showMap(%1, %2, %3);\">"
                            "<img src=\"/static/maparticle.png\"></a>")
                              .arg(coords.first).arg(coords.second).arg(zoom).toAscii();
        data += QByteArray("<a class=\"evopedianav\" href=\"") +
                backend->getOrigUrl(t).toEncoded() +
                QByteArray("\"><img src=\"/static/wikipedia.png\"></a>");
        data += extractInterLanguageLinks(articleData);
        data += "</div>";
        if (getLayoutDirection(backend->getLanguage()) == Qt::RightToLeft)
            data += "<div dir=\"rtl\">";
        if (!evopedia->networkConnectionAllowed()) {
            articleData = disableOnlineLinks(articleData);
        }
        data += articleData;
        if (getLayoutDirection(backend->getLanguage()) == Qt::RightToLeft)
            data += "</div>";
        data += getResource(":/web/footer.html");
        outputResponse(socket, data, "text/html; charset=\"utf-8\"", false);
    }
}

QByteArray &EvopediaWebServer::disableOnlineLinks(QByteArray &data)
{
    /* TODO1 completely remove the link */
    data.replace(QByteArray("src=\"http://upload.wikimedia.org/"),
                 QByteArray("src=\"/"));
    return data;
}

QByteArray EvopediaWebServer::extractInterLanguageLinks(QByteArray &data)
{
    static QRegExp rx("<a href=\"(\\./)?\\.\\./([^/]*)/([^\"]*)\">([^<]*)</a>");
    //static QRegExp startrx("<h5>[^<]*</h5>[^<]*<div class=\"pBody\">");
    //int langStart = data.lastIndexOf("<h5>Languages</h5>");
    int langStart = data.lastIndexOf("<h5>");
    if (langStart < 0) return QByteArray();
    if (data.indexOf("<div class=\"pBody\">") < 0) return QByteArray();
    int langEnd = data.indexOf("<div class=\"visualClear\"></div>", langStart);
    if (langEnd < 0) return QByteArray();

    QByteArray installedLanguages;
    QByteArray otherLanguages;

    const QString languageText = QString::fromUtf8(data.mid(langStart, langEnd - langStart).constData());

    for (int pos = 0; (pos = rx.indexIn(languageText, pos)) != -1; pos += rx.matchedLength()) {
        const QString langID(rx.cap(2));
        const QString link(rx.cap(3));
        const QString language(rx.cap(4));
        QByteArray option(QString("<option value=\"/wiki/%1/%2\">%3</option>")
                          .arg(langID).arg(link).arg(language).toUtf8());
        if (evopedia->hasLanguage(langID))
            installedLanguages += option;
        else
            otherLanguages += option;
    }

    if (installedLanguages.isEmpty() && otherLanguages.isEmpty())
        return QByteArray();

    QByteArray result("<select onchange=\"document.location.href=this.value;\">"
                      "<option value=\"\">" + tr("Other Languages").toUtf8() + "</option>");
    result += installedLanguages;
    result += "<option value=\"\">-----</option>";
    result += otherLanguages;
    result += "</select>";

    data.remove(langStart, langEnd - langStart);
    return result;
}



void EvopediaWebServer::discardClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}

void EvopediaWebServer::closeConnection(QTcpSocket *socket)
{
    socket->close();

    if (socket->state() == QTcpSocket::UnconnectedState)
        delete socket;
}

QByteArray EvopediaWebServer::getResource(const QString name)
{
    QFile f(name);
    f.open(QIODevice::ReadOnly);
    return f.readAll();
}

