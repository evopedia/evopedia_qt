#ifndef TITLE_H
#define TITLE_H

#include <QString>

class Title
{
public:
    Title();
    Title(const QByteArray &encodedTitle, const QString &language);
    const QString &getName() const { return name; }
    const QString &getLanguage() const { return language; }
    const QString getReadableName() const { return QString(name).replace('_', ' '); }
    quint8 getFileNr() const { return fileNr; }
    quint32 getBlockStart() const { return blockStart; }
    quint32 getBlockOffset() const { return blockOffset; }
    quint32 getArticleLength() const { return articleLength; }
private:
    QString name;
    QString language;
    quint8 fileNr;
    quint32 blockStart;
    quint32 blockOffset;
    quint32 articleLength;
};

#endif // TITLE_H
