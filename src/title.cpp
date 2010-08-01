#include "title.h"

#include <QDataStream>

Title::Title()
    : fileNr(0), blockStart(0), blockOffset(0), articleLength(0)
{
}

Title::Title(const QByteArray &encodedTitle, const QString &language)
    : language(language)
{
    if (encodedTitle.length() < 15)
        return;

    QByteArray escapeData(encodedTitle.left(2));
    QByteArray positionData(encodedTitle.mid(2, 13));

    QDataStream escapeDataStream(escapeData);
    escapeDataStream.setByteOrder(QDataStream::LittleEndian);

    quint16 escapes;
    escapeDataStream >> escapes;
    
    for (int i = 0; i < 13; i ++)
        if (escapes & (1 << i))
            positionData[i] = '\n';
    
    QDataStream positionDataStream(positionData);
    positionDataStream.setByteOrder(QDataStream::LittleEndian);
    
    positionDataStream >> fileNr >> blockStart >> blockOffset >> articleLength;
    
    int titleLenBytes = encodedTitle.length() - 15;
    if (titleLenBytes > 0 && encodedTitle[encodedTitle.length() - 1] == '\n')
        titleLenBytes --;
    
    name = QString::fromUtf8(encodedTitle.mid(15, titleLenBytes).constData(),
                            titleLenBytes);
}
