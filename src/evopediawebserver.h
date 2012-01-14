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

#ifndef EVOPEDIAWEBSERVER_H
#define EVOPEDIAWEBSERVER_H

#include <QTcpServer>
#include <QTextStream>
#include <QString>
#include <QUrl>

class Evopedia;

class EvopediaWebServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit EvopediaWebServer(Evopedia *evopedia);
    void incomingConnection(int socket);

signals:
    void mapViewRequested(qreal lat, qreal lon, uint zoom);
    void applicationExitRequested();

public slots:

private slots:
     void readClient();
     void discardClient();

private:
     void outputIndexPage(QTcpSocket *socket);
     void outputStatic(QTcpSocket *socket, const QStringList &pathParts);
     void redirectRandom(QTcpSocket *socket, const QStringList &pathParts);
     void outputMathImage(QTcpSocket *socket, const QStringList &pathParts);
     void outputWikiPage(QTcpSocket *socket, const QStringList &pathParts);
     void outputSettings(QTcpSocket *socket);
     void selectArchiveLocation(QTcpSocket *socket, const QString &path);
     void addArchive(QTcpSocket *socket, const QString &path);
     void outputSearchResult(QTcpSocket *socket, const QString &query, const QString &archive);

     QByteArray &disableOnlineLinks(QByteArray &data);
     QByteArray extractInterLanguageLinks(QByteArray &data);

     void outputHeaderPart(QTcpSocket *socket, const QString &key, const QString &value);
     void outputHeader(QTcpSocket *socket, const QString responseCode="200",
                       const QString contentType="text/html; charset=\"utf-8\"");
     void outputResponse(QTcpSocket *socket, const QByteArray &data,
                         const QString contentType="text/html; charset=\"utf-8\"",
                         bool cache=true);
     void outputRedirect(QTcpSocket *socket, const QUrl &to);
     void closeConnection(QTcpSocket *socket);
     QByteArray getResource(const QString name);

     Evopedia *evopedia;
};

#endif // EVOPEDIAWEBSERVER_H
