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
