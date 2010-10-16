#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H

#include <QObject>
#include <QSettings>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QHash>

#include "titleiterator.h"
#include "geotitle.h"

#include "archive.h"

class LocalArchive : public Archive
{
    Q_OBJECT
public:
    LocalArchive(const QString &directory, QObject *parent=0);
    
    TitleIterator getTitlesWithPrefix(const QString &prefix);
    QList<GeoTitle> getTitlesInCoords(const QRectF &rect, int maxTitles=-1, bool *complete=0);
    const QByteArray getArticle(const QString &title);
    const Title getTitle(const QString &title);
    const QByteArray getArticle(const Title &t);
    const Title getTitleFromPath(const QStringList &pathParts);
    QUrl getOrigUrl(const Title &title) const;
    const QString &getOrigUrl() const { return dumpOrigURL; }
    const QByteArray getMathImage(const QByteArray &hexHash) const;
    const Title getRandomTitle();

    int getNumArticles() const { return dumpNumArticles.toInt(); }
    bool isReadable() const { return readable; }

    const QString &getErrorMessage() const { return errorMessage; }

    const QString &getDirectory() const { return directory; }
    
    static const QString normalize(const QString &str);
private:
    void initializeCoords(QSettings &metadata);
    bool findMathImage(const QByteArray &hexHash, quint32 &pos, quint32 &length) const;
    void getTitlesInCoordsInt(QList<GeoTitle> &list, QFile &titles, QFile &coordFile, qint64 coordFilePos,
                                               const QRectF &targetRect, const QRectF &thisRect,
                                               int maxTitles);
    bool checkExistenceOfDumpfiles();

    const Title getTitleAtOffset(quint32 offset);

    QString errorMessage;

    bool readable;
    const QString directory;
    
    QString titleFile;
    QString mathIndexFile;
    QString mathDataFile;
    QStringList coordinateFiles;
    
    QString dumpOrigURL;
    QString dumpVersion;
    QString dumpNumArticles;
    bool normalizedTitles;


    static const QHash<QChar, QChar> *normalizationMap();
};

#endif // STORAGEBACKEND_H
