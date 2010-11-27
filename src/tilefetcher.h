#ifndef TILEFETCHER_H
#define TILEFETCHER_H

#include <QObject>
#include <QThread>
#include <QPoint>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>

class TileFetcher : public QThread
{
    Q_OBJECT

    QNetworkAccessManager *netManager;

private slots:
    void networkRequestCompleted(QNetworkReply *reply);

protected:
    void run();

public:
    explicit TileFetcher(QObject *parent = 0);

signals:
    void tileLoaded(int zoom, QPoint offset, QImage pixmap);

public slots:
    void fetchTile(int zoom, QPoint offset);

};

#endif // TILEFETCHER_H
