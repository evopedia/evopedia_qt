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

#include "evopediawebserver.h"

#include <QTcpSocket>
#include <QString>
#include <QStringBuilder>
#include <QStringList>
#include <QTextDocument>
#include <QDateTime>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QNetworkInterface>
#include <QRegExp>
#include <QLocale>
#include <QCryptographicHash>

#include "evopedia.h"
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

    /* Only read the first line.
     * Subsequent lines will only be read for individual requests
     * (for example to discover the Host-header)
     */
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
    if (pathParts.length() < 1 || pathParts[0].isEmpty()) {
        outputIndexPage(socket);
        closeConnection(socket);
        return;
    }
    const QString &firstPart = pathParts[0];
    if (firstPart == "static") {
        outputStatic(socket, pathParts);
    } else if (firstPart == "searchsuggest") {
        outputSearchSuggestion(socket, url.queryItemValue("q"), url.queryItemValue("lang"));
    } else if (firstPart == "search") {
        outputSearchResult(socket, url.queryItemValue("q"), url.queryItemValue("lang"));
    } else if (firstPart == "settings") {
        outputSettings(socket);
    } else if (firstPart == "select_archive_location") {
        selectArchiveLocation(socket, url.queryItemValue("p"));
    } else if (firstPart == "add_archive") {
        addArchive(socket, url.queryItemValue("p"));
    } else if (firstPart == "exit") {
        /* only quit via browser if we do not have a GUI */
        if (!evopedia->isGUIEnabled())
            applicationExitRequested();
        /* TODO message and error */
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
    } else if (firstPart == "opensearch") {
        if (pathParts.length() < 2) {
            outputHeader(socket, "404");
        } else {
            outputOpenSearchDescription(socket, pathParts[1]);
        }
    } else {
        outputHeader(socket, "404");
    }
    closeConnection(socket);
}

QByteArray EvopediaWebServer::getHTMLHeader()
{
    QByteArray data = getResource(":/web/header.html");
    QByteArray openSearchHeaders = "";

    foreach (QString lang, evopedia->getArchiveManager()->getDefaultLocalArchives().keys()) {
        openSearchHeaders += "<link rel=\"search\" type=\"application/opensearchdescription+xml\" "
                             "href=\"/opensearch/" + lang.toUtf8() + "\" "
                             "title=\"Evopedia (" + lang.toUtf8() + ")\" />";
    }

    data.replace("OPENSEARCHHEADERS", openSearchHeaders);
    return data;
}

void EvopediaWebServer::outputIndexPage(QTcpSocket *socket)
{
    QByteArray data = getHTMLHeader();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("random article") + "\" "
                    "href=\"/random\"><img src=\"/static/random.png\"></a>").toUtf8();
    if (!evopedia->isGUIEnabled()) {
        data += QString("<a class=\"evopedianav\" title=\"" + tr("settings") + "\" "
                        "href=\"/settings\"><img src=\"/static/settings.png\"></a>").toUtf8();
        data += QString("<a class=\"evopedianav\" title=\"" + tr("exit") + "\" "
                        "href=\"/exit\"><img src=\"/static/exit.png\"></a>").toUtf8();
    }
    data += "</div>";

    QStringList languages = evopedia->getArchiveManager()->getDefaultLocalArchives().keys();
    data += tr("<h3>Evopedia - Search</h3>").toUtf8();
    data += "<form method=\"get\" action=\"/search\" target=\"searchresult\">" +
            tr("Title:").toUtf8() +
            " <input name=\"q\"> " +
            tr("Language:").toUtf8() +
            " <select name=\"lang\">";
    foreach (QString lang, languages) {
        lang.replace("\"", "&quot;").replace("<", "&lt;");
        data += QString("<option value=\"%1\">%2</option>")
                .arg(lang, lang)
                .toUtf8();
    }
    data += "</select><input type=\"submit\" value=\"" +
            tr("Search").toUtf8() + "\">";
    if (languages.isEmpty()) {
        data += tr("<p>No archives are configured. In order to use evopedia you have "
                   "to download a Wikipedia archive. "
                   "If you do not have access to the Qt user interface of evopedia, "
                   "you have to download them manually from "
                   "<a href=\"http://dumpathome.evopedia.info/dumps/finished\">"
                   "http://dumpathome.evopedia.info/dumps/finished</a> "
                   "and install them by editing ~/.evopediarc. "
                   "Please refer to the web for more information.</p>").toUtf8();
    } else {
        QString lang(languages[0]);
        lang.replace("\"", "&quot;");
        data += "<iframe src=\"/search?q=&lang=" +
                lang.toUtf8() + "\" name=\"searchresult\" width=\"100%\" height=\"500\"></iframe>";
    }
    data += getResource(":/web/footer.html");
    outputResponse(socket, data, "text/html; charset=\"utf-8\"", false);
}

void EvopediaWebServer::outputHeader(QTcpSocket *socket, const QString responseCode, const QString contentType)
{
    /* TODO2 404 should not have "Ok" */
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
    } else if (pathParts[1] == "evopedia.js") {
            outputResponse(socket, getResource(":/web/evopedia.js"), "text/javascript");
    } else {
        outputResponse(socket, getResource(":/web/" + pathParts[1]), "image/png");
    }
}

void EvopediaWebServer::redirectRandom(QTcpSocket *socket, const QStringList &pathParts)
{
    LocalArchive *backend = 0;
    if (pathParts.length() >= 2) {
        backend = evopedia->getArchiveManager()->getLocalArchive(pathParts[1]);
    } else {
        backend = evopedia->getArchiveManager()->getRandomLocalArchive();
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
    foreach (LocalArchive *backend, evopedia->getArchiveManager()->getDefaultLocalArchives()) {
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
    LocalArchive *backend = 0;
    if (pathParts.length() >= 3)
        backend = evopedia->getArchiveManager()->getLocalArchive(pathParts[1]);
    if (backend == 0) {
        if (pathParts[1].length() > 1 && evopedia->networkConnectionAllowed() && pathParts.length() >= 3) {
            /* TODO special characters in title */
            const QUrl redirectTo(QString("http://%1.wikipedia.org/wiki/%2").arg(pathParts[1]).arg(pathParts[2]));
            outputRedirect(socket, redirectTo);
            return;
        }
        foreach (LocalArchive *b, evopedia->getArchiveManager()->getDefaultLocalArchives()) {
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

        QByteArray data = getHTMLHeader();
        data += QString("<a class=\"evopedianav\" title=\"" + tr("random article") + "\" "
                        "href=\"/random\"><img src=\"/static/random.png\"></a>").toUtf8();
        if (!evopedia->isGUIEnabled()) {
            data += QString("<a class=\"evopedianav\" title=\"" + tr("settings") + "\" "
                            "href=\"/settings\"><img src=\"/static/settings.png\"></a>").toUtf8();
            data += QString("<a class=\"evopedianav\" title=\"" + tr("exit") + "\" "
                            "href=\"/exit\"><img src=\"/static/exit.png\"></a>").toUtf8();
        }

        bool ok(false);
        int zoom;
        if (evopedia->isGUIEnabled()) {
            QPair<qreal, qreal>coords = parseCoordinatesInArticle(articleData, &ok, &zoom);
            if (ok)
                data += QString("<a class=\"evopedianav\" "
                                "href=\"#\" title=\"" + tr("show article on map") + "\" onclick=\"showMap(%1, %2, %3);\">"
                                "<img src=\"/static/maparticle.png\"></a>")
                                  .arg(coords.first).arg(coords.second).arg(zoom).toUtf8();
        }
        data += QString("<a class=\"evopedianav\" title=\"" + tr("article at Wikipedia") + "\" href=\"").toUtf8() +
                backend->getOrigUrl(t).toEncoded() +
                QByteArray("\"><img src=\"/static/wikipedia.png\"></a>");
        data += extractInterLanguageLinks(articleData);
        data += "</div>";
        if (getLayoutDirection(backend->getLanguage()) == Qt::RightToLeft)
            data += "<div dir=\"rtl\">";
        if (!evopedia->networkConnectionAllowed()) {
            articleData = disableOnlineLinks(articleData);
            data += QString("<small>" + tr("Network access disabled in application, images blocked.") + "</small>").toUtf8();
        }
        data += articleData;
        if (getLayoutDirection(backend->getLanguage()) == Qt::RightToLeft)
            data += "</div>";
        data += getResource(":/web/footer.html");
        outputResponse(socket, data, "text/html; charset=\"utf-8\"", false);
    }
}

void EvopediaWebServer::outputSettings(QTcpSocket *socket)
{
    QByteArray data = getHTMLHeader();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("search") + "\" "
                    "href=\"/\"><img src=\"/static/search.png\"></a>").toUtf8();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("random article") + "\" "
                    "href=\"/random\"><img src=\"/static/random.png\"></a>").toUtf8();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("settings") + "\" "
                    "href=\"/settings\"><img src=\"/static/settings.png\"></a>").toUtf8();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("exit") + "\" "
                    "href=\"/exit\"><img src=\"/static/exit.png\"></a>").toUtf8();
    data += "</div>";

    data += tr("<h2>Wikipedia Archives Installed:</h2>").toUtf8();
    data += QString("<table class=\"prettytable\"><tr><th>" +
                    tr("Language") + "</th><th>" + tr("Date") + "</th>" +
                    "<th>" + tr("Articles") + "</th></tr>").toUtf8();
    data += QString("<tr><td colspan=\"3\"><a href=\"/select_archive_location\">" +
                            tr("add archive") + "</a></td></tr>").toUtf8();

    QList<LocalArchive *> archives(evopedia->getArchiveManager()->getDefaultLocalArchives().values());
    qSort(archives.begin(), archives.end(), Archive::comparePointers);

    foreach (LocalArchive *a, archives) {
        data += QString("<tr><td>" + a->getLanguage() +
                        "</td><td>" + a->getDate() +
                        "</td><td>" + QString::number(a->getNumArticles()) +
                        "</td></tr>").toUtf8();
    }
    data += "</table>";

    const QString version(EVOPEDIA_VERSION);
    /* TODO define this as a macro */
    data += trUtf8("<h2>Evopedia %1</h2>"
                             "<p>Offline Wikipedia Viewer</p>"
                             "<p>Copyright Information<br/>"
                             "<small>This program shows articles from "
                             "<a href=\"http://wikipedia.org\">Wikipedia</a>, "
                             "available under the "
                             "<a href=\"http://creativecommons.org/licenses/by-sa/3.0/\">"
                             "Creative Commons Attribution/Share-Alike License</a>. "
                             "Further information can be found via the links "
                             "to the online versions of the respective "
                             "articles.</small></p>"
                             "<p>Authors<br/>"
                             "<small>"
                             "Code: Christian Reitwiessner, Joachim Schiele<br/>"
                             "Icon: Joachim Schiele<br/>"
                             "Translations: Catalan: Toni Hermoso, Czech: Veronika Kočová, "
                                       "Dutch: Daniel Ronde, French: mossroy, "
                                       "German: Christian Reitwiessner, Italian: Stefano Ravazzolo, "
                                       "Japanese: boscowitch, "
                                       "Spanish: Santiago Crespo, Vietnamese: Thuy Duong"
                   "</small></p>").arg(version).toUtf8();

    data += getResource(":/web/footer.html");
    outputResponse(socket, data, "text/html; charset=\"utf-8\"", false);
}

void EvopediaWebServer::selectArchiveLocation(QTcpSocket *socket, const QString &path)
{
    QByteArray data = getHTMLHeader();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("search") + "\" "
                    "href=\"/\"><img src=\"/static/search.png\"></a>").toUtf8();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("random article") + "\" "
                    "href=\"/random\"><img src=\"/static/random.png\"></a>").toUtf8();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("settings") + "\" "
                    "href=\"/settings\"><img src=\"/static/settings.png\"></a>").toUtf8();
    data += QString("<a class=\"evopedianav\" title=\"" + tr("exit") + "\" "
                    "href=\"/exit\"><img src=\"/static/exit.png\"></a>").toUtf8();
    data += "</div>";

    data += tr("<h2>Please Choose Archive Directory:</h2>").toUtf8();


    QDir dir(path);
    if (path == "" || !dir.exists())
        dir = QDir::root();
    data += QString("<p>" + Qt::escape(dir.canonicalPath()) + "</p>").toUtf8();

    LocalArchive *a = new LocalArchive(path);
    if (a->isReadable()) {
        data += QString("<p>" + tr("Archive for language %1 (%2)").arg(a->getLanguage(), a->getDate()) +
                        "<a href=\"/add_archive?p=" + QUrl::toPercentEncoding(path) + "\"> " +
                        tr("add this") + "</a></p>").toUtf8();
    }
    delete a;

    data += "<ul>";
    dir.setFilter(QDir::Dirs | QDir::Drives | QDir::NoDot);
    QFileInfoList subdirs = dir.entryInfoList();
    foreach (const QFileInfo &fileInfo, subdirs) {
        QString name = fileInfo.fileName();
        QString path = fileInfo.canonicalFilePath();
        data += QString("<li><a href=\"/select_archive_location?p=" +
                QUrl::toPercentEncoding(path) + "\">" +
                Qt::escape(name) + "</a></li>").toUtf8();
    }
    data += "</ul>";

    data += getResource(":/web/footer.html");
    outputResponse(socket, data, "text/html; charset=\"utf-8\"", false);
}

void EvopediaWebServer::addArchive(QTcpSocket *socket, const QString &path)
{
    LocalArchive *a = new LocalArchive(path);
    if (a->isReadable()) {
        if (evopedia->getArchiveManager()->addArchive(a)) {
            /* ownership transferred */
            outputRedirect(socket, QUrl("/settings"));
            return;
        }
    }
    delete a;
    /*TODO error message */
}

void EvopediaWebServer::outputOpenSearchDescription(QTcpSocket *socket, const QString &language)
{
    if (!evopedia->getArchiveManager()->hasLanguage(language)) {
        outputHeader(socket, "404");
        return;
    }

    /* We need the correct absolute URL. For this, we have to read the
     * Host-header.
     * This only works if the client sends the request out as a single packet.
     */

    /* good guess if it does not work */
    QByteArray url = "http://localhost:8080";
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        if (line.length() == 0) break;
        if (line.startsWith("\r\n")) break;
        if (line.mid(0, 6).toUpper() == "HOST: ") {
            url = ("http://" + line.mid(6)).trimmed();
            break;
        }
    }

    QByteArray data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
            "<OpenSearchDescription xmlns=\"http://a9.com/-/spec/opensearch/1.1/\">"
              "<ShortName>Evopedia Search (" + language.toUtf8() + ")</ShortName>"
              "<Description>Search offline articles in Evopedia.</Description>"
            "<Url type=\"text/html\" method=\"GET\" template=\"" +
            url + "/search?q={searchTerms}&amp;lang=" +
            language.toUtf8() + "&amp;format=rss\" />" +
            "<Url type=\"application/x-suggestions+json\" template=\"" +
            url + "/searchsuggest?q={searchTerms}&amp;lang=" +
            language.toUtf8() + "\" />" +
            "</OpenSearchDescription>";
        outputResponse(socket, data, "application/opensearchdescription+xml");
}

void EvopediaWebServer::outputSearchSuggestion(QTcpSocket *socket, const QString &query, const QString &archive)
{
    LocalArchive *a = evopedia->getArchiveManager()->getLocalArchive(archive);
    if (!a) {
        outputHeader(socket, "404");
        return;
    }

    TitleIterator it(a->getTitlesWithPrefix(query));
    QStringList items;
    for (int titles = 0; it.hasNext() && titles < 10; titles ++) {
        Title t(it.next());
        items += jsonEncodeString(t.getReadableName());
    }

    QString data = "[" + jsonEncodeString(query) + ", [" + items.join(", ") + "]]";

    outputResponse(socket, data.toUtf8(), "application/json; charset=\"utf-8\"");
}

void EvopediaWebServer::outputSearchResult(QTcpSocket *socket, const QString &query, const QString &archive)
{
    QByteArray data = getHTMLHeader() + "</div>";
    LocalArchive *a = evopedia->getArchiveManager()->getLocalArchive(archive);
    if (a) {
        TitleIterator it(a->getTitlesWithPrefix(query));
        if (!it.hasNext()) {
            data += tr("Nothing found.").toUtf8();
        } else {
            for (int titles = 0; it.hasNext() && titles < 50; titles ++) {
                Title t(it.next());
                data += QString("<a href=\"%1\" target=\"_top\">%2</a><br/>")
                        .arg(evopedia->getArticleUrl(t).toEncoded(),
                             t.getReadableName()).toUtf8();
            }
        }
    } else {
        data += tr("Unknown archive/language.").toUtf8();
    }
    data += getResource(":web/footer.html");
    outputResponse(socket, data, "text/html; charset=\"utf-8\"", false);
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
        if (evopedia->getArchiveManager()->hasLanguage(langID))
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

