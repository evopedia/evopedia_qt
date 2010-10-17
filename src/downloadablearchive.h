#ifndef DOWNLOADABLEARCHIVE_H
#define DOWNLOADABLEARCHIVE_H

#include <QUrl>
#include <QNetworkReply>

#include "archive.h"


class DownloadableArchive : public Archive
{
    Q_OBJECT

    QUrl url;
    QString size;

    /* only used and initialized after download started */
    QString downloadDirectory;
    QString torrentFile;

private slots:
    void torrentDownloadFinished(QNetworkReply* reply);

public:
    explicit DownloadableArchive(const QString &language, const QString &date,
                                 const QUrl &url, const QString &size, QObject *parent = 0);
    const QString &getSize() const { return size; }

signals:

public slots:

    bool startDownload();

};

#endif // DOWNLOADABLEARCHIVE_H
