#include "storagebackend.h"

#include <iostream>

#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QUrl>

#include <bzlib.h>

#include "title.h"
#include "bzreader.h"
#include "utils.h"

QPointF readCoordinate(QDataStream &s)
{
    float lat, lon;
    s >> lat;
    s >> lon;
    return QPointF(lon, lat);
}


StorageBackend::StorageBackend(const QString &directory, QObject *parent) :
        QObject(parent),
        readable(false),
        directory(directory),
        titleFile(directory + QString("/titles.idx")),
        mathIndexFile(directory + QString("/math.idx")),
        mathDataFile(directory + QString("/math.dat")),
        normalizedTitles(true)
{
    if (!checkExistenceOfDumpfiles()) {
        readable = false;
        return;
    }
    QFile titles(titleFile);
    titles.open(QIODevice::ReadOnly);
    if (!titles.isReadable()) {
        errorMessage = "Title file not readable.";
        readable = false;
        return;
    } else {
        readable = true;
    }

    QSettings metadata(directory + QString("/metadata.txt"), QSettings::IniFormat);
    dumpDate = metadata.value("dump/date").toString();
    dumpLanguage = metadata.value("dump/language").toString();
    dumpOrigURL = metadata.value("dump/orig_url").toString();
    dumpVersion = metadata.value("dump/version").toString();
    dumpNumArticles = metadata.value("dump/num_articles").toString();
    normalizedTitles = metadata.value("dump/normalized_titles", true).toBool();
    initializeCoords(metadata);

    readable = true;
}

bool StorageBackend::checkExistenceOfDumpfiles()
{
    QDir dir(directory);
    if (!dir.exists()) {
        errorMessage = "Dump directory does not exist.";
        return false;
    }
    if (!QFile(directory + QString("/metadata.txt")).exists()) {
        errorMessage = "Metadata file does not exist.";
        return false;
    }
    if (!QFile(titleFile).exists()) {
        errorMessage = "Title file does not exist.";
        return false;
    }
    return true;
}

TitleIterator StorageBackend::getTitlesWithPrefix(const QString &prefix)
{
    QFile *titles = new QFile(titleFile);
    if (!titles->open(QIODevice::ReadOnly))
        return TitleIterator();

    int lo = 0;
    int hi = titles->size();

    QString prefix_normalized;

    if (normalizedTitles) {
        prefix_normalized = normalize(prefix);
    } else {
        prefix_normalized = prefix;
    }

    while (lo < hi) {
        int mid = (lo + hi) / 2;
        titles->seek(mid);
        QByteArray line = titles->readLine();
        int aftermid = mid + line.length();
        if (mid > 0) { /* potentially incomplete line */
            line = titles->readLine();
            aftermid += line.length();
        }
        if (line.size() == 0) { /* end of file */
            hi = mid;
            continue;
        }
        Title title(line.left(line.length() - 1), QString());
        const QString nt(normalizedTitles ? normalize(title.getName()) : title.getName());
        if (nt < prefix_normalized) {
            lo = aftermid - 1;
        } else {
            hi = mid;
        }
    }
    if (lo > 0) {
        /* let lo point to the start of an entry */
        lo ++;
    }
    titles->seek(lo);
    return TitleIterator(titles, prefix, dumpLanguage);
}

QList<GeoTitle> StorageBackend::getTitlesInCoords(const QRectF &rect, int maxTitles)
{
    QRectF rectN(rect.normalized());

    QList<GeoTitle> list;

    QFile titles(titleFile);
    titles.open(QIODevice::ReadOnly);

    foreach (QString f, coordinateFiles) {
        QFile cf(f);
        if (!cf.open(QIODevice::ReadOnly))
            continue;

        QRectF globe(-181, -90.0, 361.0, 181.0);
        getTitlesInCoordsInt(list, titles, cf, 0, rectN, globe, maxTitles);
        if (list.length() >= maxTitles)
            return list;
    }
    return list;
}

const Title StorageBackend::getTitle(const QString &title)
{
    /* TODO1 not very efficient: we should be able to stop once the
     * normalized title in the list is longer than our normalized title */
    TitleIterator it = getTitlesWithPrefix(title);
    while (it.hasNext()) {
        Title t(it.next());
        if (t.getName() == title) {
            return t;
        }
    }
    return Title();
}

const QByteArray StorageBackend::getArticle(const QString &title)
{
    Title t = getTitle(title);
    if (t.getName().isEmpty()) {
        return QByteArray();
    } else {
        return getArticle(t);
    }
}

const QByteArray StorageBackend::getArticle(const Title &t)
{
    Title title(t);
    if (title.getFileNr() == 0xff) { /* redirect */
        if (title.getBlockStart() == 0xffffffL) {
            return QByteArray(); /* invalid redirect */
        } else {
            title = getTitleAtOffset(title.getBlockStart());
        }
    }
    QString fileNumber;
    fileNumber.sprintf("%02d", uint(title.getFileNr()));
    QString fileName(QString("%1/wikipedia_%2.dat").arg(directory).arg(fileNumber));
    std::cout << fileName.toStdString() << "\n" << std::flush;
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly))
        return QByteArray();

    BZReader reader;
    return reader.readAt(f,
                         title.getBlockStart(),
                         title.getBlockOffset(),
                         title.getArticleLength());
}

const Title StorageBackend::getTitleFromPath(const QStringList &pathParts)
{
    static QRegExp endpattern("(_[0-9a-f]{4})?(\\.html(\\.redir)?)?$", Qt::CaseInsensitive);
    QString t;
    if (dumpVersion == "0.2") {
        t = pathParts[pathParts.length() - 1];
        t.replace(endpattern, "");
    } else {
        t = pathParts[pathParts.length() - 1];
    }
    return getTitle(t);
}

const Title StorageBackend::getRandomTitle()
{
    /* long titles are preferred by this method, oh well... */

    QFile titles(titleFile);
    if (!titles.open(QIODevice::ReadOnly))
        return Title();

    int pos = randomNumber(titles.size());
    titles.seek(pos);
    titles.readLine();
    if (titles.atEnd())
        titles.seek(0);
    return Title(titles.readLine(), dumpLanguage);
}

QUrl StorageBackend::getOrigUrl(const Title &title) const
{
    return QUrl(dumpOrigURL + title.getName().toUtf8());
}

void StorageBackend::initializeCoords(QSettings &metadata)
{
    coordinateFiles = QStringList();
    for (int i = 1; ; i ++) {
        QString key = QString().sprintf("coordinates/file_%02d", uint(i));
        if (!metadata.contains(key))
            break;
        coordinateFiles += QString("%1/%2").arg(directory, metadata.value(key).toString());
    }
}



bool StorageBackend::findMathImage(const QByteArray &hexHash, quint32 &pos, quint32 &length) const
{
    QFile f(mathIndexFile);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    const int entrysize = 16 + 4 + 4;
    int lo = 0;
    int hi = f.size() / entrysize;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        f.seek(mid * entrysize);
        QByteArray entry = f.read(entrysize);
        QByteArray entryHash = entry.left(16);
        if (entryHash == hexHash) {
            QDataStream entryDataStream(entry.mid(16));
            entryDataStream.setByteOrder(QDataStream::LittleEndian);
            entryDataStream >> pos >> length;
            return true;
        } else if (hexHash < entryHash) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return false;
}

void StorageBackend::getTitlesInCoordsInt(QList<GeoTitle> &list, QFile &titles, QFile &coordFile, qint64 coordFilePos,
                                          const QRectF &targetRect, const QRectF &thisRect, int maxTitles)
{
    if (maxTitles >= 0 && list.length() >= maxTitles)
        return;

    coordFile.seek(coordFilePos);
    QDataStream s(&coordFile);
    s.setByteOrder(QDataStream::LittleEndian);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint16 selector;
    s >> selector;
    if (selector == 0xffff) {
        /* not enough articles, further subdivision needed */
        QPointF center = readCoordinate(s);
        quint32 lensw, lense, lennw;
        s >> lensw >> lense >> lennw;
        qint64 pos0 = coordFilePos + 22;
        qint64 pos1 = pos0 + lensw;
        qint64 pos2 = pos1 + lense;
        qint64 pos3 = pos2 + lennw;

        /* the y coordinate is inverted in QRectF! */

        QRectF rectSW(thisRect.topLeft(), center);
        if (targetRect.intersects(rectSW))
            getTitlesInCoordsInt(list, titles, coordFile, pos0, targetRect, rectSW, maxTitles);

        QRectF rectSE(QRectF(thisRect.topRight(), center).normalized());
        if (targetRect.intersects(rectSE))
            getTitlesInCoordsInt(list, titles, coordFile, pos1, targetRect, rectSE, maxTitles);

        QRectF rectNW(QRectF(thisRect.bottomLeft(), center).normalized());
        if (targetRect.intersects(rectNW))
            getTitlesInCoordsInt(list, titles, coordFile, pos2, targetRect, rectNW, maxTitles);

        QRectF rectNE(center, thisRect.bottomRight());
        if (targetRect.intersects(rectNE))
            getTitlesInCoordsInt(list, titles, coordFile, pos3, targetRect, rectNE, maxTitles);
    } else {
        for (int i = 0; i < selector; i ++) {
            QPointF c = readCoordinate(s);
            quint32 title_pos;
            s >> title_pos;
            if (!targetRect.contains(c))
                continue;
            titles.seek(title_pos);
            list += GeoTitle(Title(titles.readLine(), dumpLanguage), c);
            if (maxTitles >= 0 && list.length() >= maxTitles)
                return;
        }

    }
}


const QByteArray StorageBackend::getMathImage(const QByteArray &hexHash) const
{
    if (hexHash.length() != 16)
        return QByteArray();

    quint32 pos, length;
    if (!findMathImage(hexHash, pos, length))
        return QByteArray();

    QFile f(mathDataFile);
    if (!f.open(QIODevice::ReadOnly))
        return QByteArray();

    f.seek(pos);
    return f.read(length);
}


const Title StorageBackend::getTitleAtOffset(quint32 offset)
{
    QFile titles(titleFile);
    titles.open(QIODevice::ReadOnly);
    titles.seek(offset);
    return Title(titles.readLine(), dumpLanguage);
}

const QHash<QChar,QChar> *StorageBackend::normalizationMap()
{
    static QHash<QChar,QChar> nm;
    if (nm.empty()) {
#if 0
        nm[L'Ḅ'] = 'b'; nm[L'Ć'] = 'c'; nm[L'Ȍ'] = 'o'; nm[L'ẏ'] = 'y'; nm[L'Ḕ'] = 'e'; nm[L'Ė'] = 'e';
        nm[L'ơ'] = 'o'; nm[L'Ḥ'] = 'h'; nm[L'Ȭ'] = 'o'; nm[L'ắ'] = 'a'; nm[L'Ḵ'] = 'k'; nm[L'Ķ'] = 'k';
        nm[L'ế'] = 'e'; nm[L'Ṅ'] = 'n'; nm[L'ņ'] = 'n'; nm[L'Ë'] = 'e'; nm[L'ỏ'] = 'o'; nm[L'Ǒ'] = 'o';
        nm[L'Ṕ'] = 'p'; nm[L'Ŗ'] = 'r'; nm[L'Û'] = 'u'; nm[L'ở'] = 'o'; nm[L'ǡ'] = 'a'; nm[L'Ṥ'] = 's';
        nm[L'ë'] = 'e'; nm[L'ữ'] = 'u'; nm[L'p'] = 'p'; nm[L'Ṵ'] = 'u'; nm[L'Ŷ'] = 'y'; nm[L'û'] = 'u';
        nm[L'ā'] = 'a'; nm[L'Ẅ'] = 'w'; nm[L'ȇ'] = 'e'; nm[L'ḏ'] = 'd'; nm[L'ȗ'] = 'u'; nm[L'ḟ'] = 'f';
        nm[L'ġ'] = 'g'; nm[L'Ấ'] = 'a'; nm[L'ȧ'] = 'a'; nm[L'ḯ'] = 'i'; nm[L'Ẵ'] = 'a'; nm[L'ḿ'] = 'm';
        nm[L'À'] = 'a'; nm[L'Ễ'] = 'e'; nm[L'ṏ'] = 'o'; nm[L'ő'] = 'o'; nm[L'Ổ'] = 'o'; nm[L'ǖ'] = 'u';
        nm[L'ṟ'] = 'r'; nm[L'š'] = 's'; nm[L'à'] = 'a'; nm[L'Ụ'] = 'u'; nm[L'Ǧ'] = 'g'; nm[L'k'] = 'k';
        nm[L'ṯ'] = 't'; nm[L'ű'] = 'u'; nm[L'Ỵ'] = 'y'; nm[L'ṿ'] = 'v'; nm[L'Ā'] = 'a'; nm[L'Ȃ'] = 'a';
        nm[L'ẅ'] = 'w'; nm[L'Ḋ'] = 'd'; nm[L'Ȓ'] = 'r'; nm[L'Ḛ'] = 'e'; nm[L'Ġ'] = 'g'; nm[L'ấ'] = 'a';
        nm[L'Ḫ'] = 'h'; nm[L'İ'] = 'i'; nm[L'Ȳ'] = 'y'; nm[L'ẵ'] = 'a'; nm[L'Ḻ'] = 'l'; nm[L'Á'] = 'a';
        nm[L'ễ'] = 'e'; nm[L'Ṋ'] = 'n'; nm[L'Ñ'] = 'n'; nm[L'Ő'] = 'o'; nm[L'ổ'] = 'o'; nm[L'Ǜ'] = 'u';
        nm[L'Ṛ'] = 'r'; nm[L'á'] = 'a'; nm[L'Š'] = 's'; nm[L'ụ'] = 'u'; nm[L'f'] = 'f'; nm[L'ǫ'] = 'o';
        nm[L'Ṫ'] = 't'; nm[L'ñ'] = 'n'; nm[L'Ű'] = 'u'; nm[L'ỵ'] = 'y'; nm[L'v'] = 'v'; nm[L'ǻ'] = 'a';
        nm[L'Ṻ'] = 'u'; nm[L'ḅ'] = 'b'; nm[L'ċ'] = 'c'; nm[L'Ẋ'] = 'x'; nm[L'ȍ'] = 'o'; nm[L'ḕ'] = 'e';
        nm[L'ě'] = 'e'; nm[L'Ơ'] = 'o'; nm[L'ḥ'] = 'h'; nm[L'ī'] = 'i'; nm[L'Ẫ'] = 'a'; nm[L'ȭ'] = 'o';
        nm[L'ư'] = 'u'; nm[L'ḵ'] = 'k'; nm[L'Ļ'] = 'l'; nm[L'Ẻ'] = 'e'; nm[L'ṅ'] = 'n'; nm[L'Ị'] = 'i';
        nm[L'ǐ'] = 'i'; nm[L'ṕ'] = 'p'; nm[L'Ö'] = 'o'; nm[L'ś'] = 's'; nm[L'Ớ'] = 'o'; nm[L'a'] = 'a';
        nm[L'Ǡ'] = 'a'; nm[L'ṥ'] = 's'; nm[L'ū'] = 'u'; nm[L'Ừ'] = 'u'; nm[L'q'] = 'q'; nm[L'ǰ'] = 'j';
        nm[L'ṵ'] = 'u'; nm[L'ö'] = 'o'; nm[L'Ż'] = 'z'; nm[L'Ȁ'] = 'a'; nm[L'ẃ'] = 'w'; nm[L'Ă'] = 'a';
        nm[L'Ḉ'] = 'c'; nm[L'Ȑ'] = 'r'; nm[L'Ē'] = 'e'; nm[L'Ḙ'] = 'e'; nm[L'ả'] = 'a'; nm[L'Ģ'] = 'g';
        nm[L'Ḩ'] = 'h'; nm[L'Ȱ'] = 'o'; nm[L'ẳ'] = 'a'; nm[L'Ḹ'] = 'l'; nm[L'ể'] = 'e'; nm[L'Ç'] = 'c';
        nm[L'Ṉ'] = 'n'; nm[L'Ǎ'] = 'a'; nm[L'ồ'] = 'o'; nm[L'Ṙ'] = 'r'; nm[L'ợ'] = 'o'; nm[L'Ţ'] = 't';
        nm[L'ç'] = 'c'; nm[L'Ṩ'] = 's'; nm[L'ǭ'] = 'o'; nm[L'l'] = 'l'; nm[L'ỳ'] = 'y'; nm[L'Ų'] = 'u';
        nm[L'Ṹ'] = 'u'; nm[L'ḃ'] = 'b'; nm[L'Ẉ'] = 'w'; nm[L'ȋ'] = 'i'; nm[L'č'] = 'c'; nm[L'ḓ'] = 'd';
        nm[L'ẘ'] = 'w'; nm[L'ț'] = 't'; nm[L'ĝ'] = 'g'; nm[L'ḣ'] = 'h'; nm[L'Ẩ'] = 'a'; nm[L'ȫ'] = 'o';
        nm[L'ĭ'] = 'i'; nm[L'ḳ'] = 'k'; nm[L'Ẹ'] = 'e'; nm[L'Ľ'] = 'l'; nm[L'ṃ'] = 'm'; nm[L'Ỉ'] = 'i';
        nm[L'ō'] = 'o'; nm[L'Ì'] = 'i'; nm[L'ṓ'] = 'o'; nm[L'ǒ'] = 'o'; nm[L'Ộ'] = 'o'; nm[L'ŝ'] = 's';
        nm[L'Ü'] = 'u'; nm[L'ṣ'] = 's'; nm[L'g'] = 'g'; nm[L'Ứ'] = 'u'; nm[L'ŭ'] = 'u'; nm[L'ì'] = 'i';
        nm[L'ṳ'] = 'u'; nm[L'w'] = 'w'; nm[L'Ỹ'] = 'y'; nm[L'Ž'] = 'z'; nm[L'ü'] = 'u'; nm[L'Ȇ'] = 'e';
        nm[L'ẉ'] = 'w'; nm[L'Č'] = 'c'; nm[L'Ḏ'] = 'd'; nm[L'Ȗ'] = 'u'; nm[L'ẙ'] = 'y'; nm[L'Ĝ'] = 'g';
        nm[L'Ḟ'] = 'f'; nm[L'Ȧ'] = 'a'; nm[L'ẩ'] = 'a'; nm[L'Ĭ'] = 'i'; nm[L'Ḯ'] = 'i'; nm[L'ẹ'] = 'e';
        nm[L'ļ'] = 'l'; nm[L'Ḿ'] = 'm'; nm[L'ỉ'] = 'i'; nm[L'Í'] = 'i'; nm[L'Ō'] = 'o'; nm[L'Ṏ'] = 'o';
        nm[L'Ǘ'] = 'u'; nm[L'ộ'] = 'o'; nm[L'Ý'] = 'y'; nm[L'Ŝ'] = 's'; nm[L'Ṟ'] = 'r'; nm[L'b'] = 'b';
        nm[L'ǧ'] = 'g'; nm[L'ứ'] = 'u'; nm[L'í'] = 'i'; nm[L'Ŭ'] = 'u'; nm[L'Ṯ'] = 't'; nm[L'r'] = 'r';
        nm[L'ỹ'] = 'y'; nm[L'ý'] = 'y'; nm[L'ż'] = 'z'; nm[L'Ṿ'] = 'v'; nm[L'ȁ'] = 'a'; nm[L'ć'] = 'c';
        nm[L'ḉ'] = 'c'; nm[L'Ẏ'] = 'y'; nm[L'ȑ'] = 'r'; nm[L'ė'] = 'e'; nm[L'ḙ'] = 'e'; nm[L'ḩ'] = 'h';
        nm[L'Ắ'] = 'a'; nm[L'ȱ'] = 'o'; nm[L'ķ'] = 'k'; nm[L'ḹ'] = 'l'; nm[L'Ế'] = 'e'; nm[L'Â'] = 'a';
        nm[L'Ň'] = 'n'; nm[L'ṉ'] = 'n'; nm[L'Ỏ'] = 'o'; nm[L'Ò'] = 'o'; nm[L'ŗ'] = 'r'; nm[L'ṙ'] = 'r';
        nm[L'ǜ'] = 'u'; nm[L'Ở'] = 'o'; nm[L'â'] = 'a'; nm[L'ṩ'] = 's'; nm[L'm'] = 'm'; nm[L'Ǭ'] = 'o';
        nm[L'Ữ'] = 'u'; nm[L'ò'] = 'o'; nm[L'ŷ'] = 'y'; nm[L'ṹ'] = 'u'; nm[L'Ȅ'] = 'e'; nm[L'ẇ'] = 'w';
        nm[L'Ḍ'] = 'd'; nm[L'Ď'] = 'd'; nm[L'Ȕ'] = 'u'; nm[L'ẗ'] = 't'; nm[L'Ḝ'] = 'e'; nm[L'Ğ'] = 'g';
        nm[L'ầ'] = 'a'; nm[L'Ḭ'] = 'i'; nm[L'Į'] = 'i'; nm[L'ặ'] = 'a'; nm[L'Ḽ'] = 'l'; nm[L'ľ'] = 'l';
        nm[L'Ã'] = 'a'; nm[L'ệ'] = 'e'; nm[L'Ṍ'] = 'o'; nm[L'Ŏ'] = 'o'; nm[L'Ó'] = 'o'; nm[L'ỗ'] = 'o';
        nm[L'Ǚ'] = 'u'; nm[L'Ṝ'] = 'r'; nm[L'Ş'] = 's'; nm[L'ã'] = 'a'; nm[L'ủ'] = 'u'; nm[L'ǩ'] = 'k';
        nm[L'h'] = 'h'; nm[L'Ṭ'] = 't'; nm[L'Ů'] = 'u'; nm[L'ó'] = 'o'; nm[L'ỷ'] = 'y'; nm[L'ǹ'] = 'n';
        nm[L'x'] = 'x'; nm[L'Ṽ'] = 'v'; nm[L'ž'] = 'z'; nm[L'ḇ'] = 'b'; nm[L'ĉ'] = 'c'; nm[L'Ẍ'] = 'x';
        nm[L'ȏ'] = 'o'; nm[L'ḗ'] = 'e'; nm[L'ę'] = 'e'; nm[L'ȟ'] = 'h'; nm[L'ḧ'] = 'h'; nm[L'ĩ'] = 'i';
        nm[L'Ậ'] = 'a'; nm[L'ȯ'] = 'o'; nm[L'ḷ'] = 'l'; nm[L'Ĺ'] = 'l'; nm[L'Ẽ'] = 'e'; nm[L'ṇ'] = 'n';
        nm[L'È'] = 'e'; nm[L'Ọ'] = 'o'; nm[L'ǎ'] = 'a'; nm[L'ṗ'] = 'p'; nm[L'ř'] = 'r'; nm[L'Ờ'] = 'o';
        nm[L'Ǟ'] = 'a'; nm[L'c'] = 'c'; nm[L'ṧ'] = 's'; nm[L'ũ'] = 'u'; nm[L'è'] = 'e'; nm[L'Ử'] = 'u';
        nm[L's'] = 's'; nm[L'ṷ'] = 'u'; nm[L'Ź'] = 'z'; nm[L'Ḃ'] = 'b'; nm[L'Ĉ'] = 'c'; nm[L'Ȋ'] = 'i';
        nm[L'ẍ'] = 'x'; nm[L'Ḓ'] = 'd'; nm[L'Ę'] = 'e'; nm[L'Ț'] = 't'; nm[L'쎟'] = 's'; nm[L'Ḣ'] = 'h';
        nm[L'Ĩ'] = 'i'; nm[L'Ȫ'] = 'o'; nm[L'ậ'] = 'a'; nm[L'Ḳ'] = 'k'; nm[L'ẽ'] = 'e'; nm[L'Ṃ'] = 'm';
        nm[L'É'] = 'e'; nm[L'ň'] = 'n'; nm[L'ọ'] = 'o'; nm[L'Ǔ'] = 'u'; nm[L'Ṓ'] = 'o'; nm[L'Ù'] = 'u';
        nm[L'Ř'] = 'r'; nm[L'ờ'] = 'o'; nm[L'Ṣ'] = 's'; nm[L'é'] = 'e'; nm[L'Ũ'] = 'u'; nm[L'ử'] = 'u';
        nm[L'n'] = 'n'; nm[L'Ṳ'] = 'u'; nm[L'ù'] = 'u'; nm[L'Ÿ'] = 'y'; nm[L'ă'] = 'a'; nm[L'Ẃ'] = 'w';
        nm[L'ȅ'] = 'e'; nm[L'ḍ'] = 'd'; nm[L'ē'] = 'e'; nm[L'ȕ'] = 'u'; nm[L'ḝ'] = 'e'; nm[L'ģ'] = 'g';
        nm[L'Ả'] = 'a'; nm[L'ḭ'] = 'i'; nm[L'Ẳ'] = 'a'; nm[L'ḽ'] = 'l'; nm[L'Ń'] = 'n'; nm[L'Ể'] = 'e';
        nm[L'ṍ'] = 'o'; nm[L'Î'] = 'i'; nm[L'Ồ'] = 'o'; nm[L'ǘ'] = 'u'; nm[L'ṝ'] = 'r'; nm[L'ţ'] = 't';
        nm[L'Ợ'] = 'o'; nm[L'i'] = 'i'; nm[L'Ǩ'] = 'k'; nm[L'ṭ'] = 't'; nm[L'î'] = 'i'; nm[L'ų'] = 'u';
        nm[L'Ỳ'] = 'y'; nm[L'y'] = 'y'; nm[L'Ǹ'] = 'n'; nm[L'ṽ'] = 'v'; nm[L'Ḁ'] = 'a'; nm[L'Ȉ'] = 'i';
        nm[L'ẋ'] = 'x'; nm[L'Ċ'] = 'c'; nm[L'Ḑ'] = 'd'; nm[L'Ș'] = 's'; nm[L'Ě'] = 'e'; nm[L'Ḡ'] = 'g';
        nm[L'Ȩ'] = 'e'; nm[L'ẫ'] = 'a'; nm[L'Ī'] = 'i'; nm[L'Ḱ'] = 'k'; nm[L'ẻ'] = 'e'; nm[L'ĺ'] = 'l';
        nm[L'Ṁ'] = 'm'; nm[L'ị'] = 'i'; nm[L'Ï'] = 'i'; nm[L'Ṑ'] = 'o'; nm[L'Ǖ'] = 'u'; nm[L'ớ'] = 'o';
        nm[L'Ś'] = 's'; nm[L'ß'] = 's'; nm[L'Ṡ'] = 's'; nm[L'd'] = 'd'; nm[L'ừ'] = 'u'; nm[L'Ū'] = 'u';
        nm[L'ï'] = 'i'; nm[L'Ṱ'] = 't'; nm[L'ǵ'] = 'g'; nm[L't'] = 't'; nm[L'ź'] = 'z'; nm[L'ÿ'] = 'y';
        nm[L'Ẁ'] = 'w'; nm[L'ȃ'] = 'a'; nm[L'ą'] = 'a'; nm[L'ḋ'] = 'd'; nm[L'ȓ'] = 'r'; nm[L'ĕ'] = 'e';
        nm[L'ḛ'] = 'e'; nm[L'Ạ'] = 'a'; nm[L'ĥ'] = 'h'; nm[L'ḫ'] = 'h'; nm[L'Ằ'] = 'a'; nm[L'ȳ'] = 'y';
        nm[L'ĵ'] = 'j'; nm[L'ḻ'] = 'l'; nm[L'Ề'] = 'e'; nm[L'Ņ'] = 'n'; nm[L'Ä'] = 'a'; nm[L'ṋ'] = 'n';
        nm[L'Ố'] = 'o'; nm[L'ŕ'] = 'r'; nm[L'Ô'] = 'o'; nm[L'ṛ'] = 'r'; nm[L'ǚ'] = 'u'; nm[L'Ỡ'] = 'o';
        nm[L'ť'] = 't'; nm[L'ä'] = 'a'; nm[L'ṫ'] = 't'; nm[L'Ǫ'] = 'o'; nm[L'o'] = 'o'; nm[L'Ự'] = 'u';
        nm[L'ŵ'] = 'w'; nm[L'ô'] = 'o'; nm[L'ṻ'] = 'u'; nm[L'Ǻ'] = 'a'; nm[L'ẁ'] = 'w'; nm[L'Ą'] = 'a';
        nm[L'Ḇ'] = 'b'; nm[L'Ȏ'] = 'o'; nm[L'Ĕ'] = 'e'; nm[L'Ḗ'] = 'e'; nm[L'Ȟ'] = 'h'; nm[L'ạ'] = 'a';
        nm[L'Ĥ'] = 'h'; nm[L'Ḧ'] = 'h'; nm[L'Ư'] = 'u'; nm[L'Ȯ'] = 'o'; nm[L'ằ'] = 'a'; nm[L'Ĵ'] = 'j';
        nm[L'Ḷ'] = 'l'; nm[L'ề'] = 'e'; nm[L'Å'] = 'a'; nm[L'ń'] = 'n'; nm[L'Ṇ'] = 'n'; nm[L'Ǐ'] = 'i';
        nm[L'ố'] = 'o'; nm[L'Õ'] = 'o'; nm[L'Ŕ'] = 'r'; nm[L'Ṗ'] = 'p'; nm[L'ǟ'] = 'a'; nm[L'ỡ'] = 'o';
        nm[L'å'] = 'a'; nm[L'Ť'] = 't'; nm[L'Ṧ'] = 's'; nm[L'j'] = 'j'; nm[L'ự'] = 'u'; nm[L'õ'] = 'o';
        nm[L'Ŵ'] = 'w'; nm[L'Ṷ'] = 'u'; nm[L'z'] = 'z'; nm[L'ḁ'] = 'a'; nm[L'Ẇ'] = 'w'; nm[L'ȉ'] = 'i';
        nm[L'ď'] = 'd'; nm[L'ḑ'] = 'd'; nm[L'ẖ'] = 'h'; nm[L'ș'] = 's'; nm[L'ğ'] = 'g'; nm[L'ḡ'] = 'g';
        nm[L'Ầ'] = 'a'; nm[L'ȩ'] = 'e'; nm[L'į'] = 'i'; nm[L'ḱ'] = 'k'; nm[L'Ặ'] = 'a'; nm[L'ṁ'] = 'm';
        nm[L'Ệ'] = 'e'; nm[L'Ê'] = 'e'; nm[L'ŏ'] = 'o'; nm[L'ṑ'] = 'o'; nm[L'ǔ'] = 'u'; nm[L'Ỗ'] = 'o';
        nm[L'Ú'] = 'u'; nm[L'ş'] = 's'; nm[L'ṡ'] = 's'; nm[L'e'] = 'e'; nm[L'Ủ'] = 'u'; nm[L'ê'] = 'e';
        nm[L'ů'] = 'u'; nm[L'ṱ'] = 't'; nm[L'u'] = 'u'; nm[L'Ǵ'] = 'g'; nm[L'Ỷ'] = 'y'; nm[L'ú'] = 'u';
        nm[L'0'] = '0'; nm[L'1'] = '1'; nm[L'2'] = '2'; nm[L'3'] = '3'; nm[L'4'] = '4'; nm[L'5'] = '5';
        nm[L'6'] = '6'; nm[L'7'] = '7'; nm[L'8'] = '8'; nm[L'9'] = '9';

        QFile f("/tmp/transtbl.dat");
        f.open(QIODevice::ReadWrite);
        QDataStream s(&f);
        QHash<QChar, QChar>::const_iterator i = nm.constBegin();
        while (i != nm.constEnd()) {
            s << quint32(i.key().unicode()) << quint32(i.value().unicode());
            ++i;
        }
#else
        QFile f(":/transtbl/transtbl.dat");
        f.open(QIODevice::ReadOnly);
        QDataStream s(&f);
        while (!s.atEnd()) {
            quint32 key, value;
            s >> key >> value;
            nm[QChar(key)] = QChar(value);
        }
#endif
    }
    return &nm;
}

const QString StorageBackend::normalize(const QString &str)
{
    QString normStr = str;
    const QHash<QChar,QChar> *nm = normalizationMap();
    for (int i = 0; i < normStr.length(); i ++) {
        QChar c = str[i].toLower();
        if (nm->contains(c))
            normStr[i] = (*nm)[c];
        else
            normStr[i] = '_';
    }
    return normStr;
}
