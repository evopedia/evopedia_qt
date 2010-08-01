#ifndef BZREADER_H
#define BZREADER_H

#include <QByteArray>
#include <QFile>

#include <bzlib.h>

class BZReader
{
public:
    BZReader();
    const QByteArray readAt(QFile &f, quint32 blockStart, quint32 blockOffset, quint32 dataLength);
private:
    bz_stream stream;
};

#endif // BZREADER_H
