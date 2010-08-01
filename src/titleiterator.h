#ifndef TITLEITERATOR_H
#define TITLEITERATOR_H

#include <QIODevice>
#include <QString>

#include "title.h"

class TitleIterator
{
public:
    TitleIterator();
    TitleIterator(QIODevice *device, const QString &prefix=QString(), const QString &language=QString());
    bool hasNext() const;
    const Title next();
private:
    void checkHasNext();

    QString language;
    QIODevice *device;
    QString prefix;
    Title nextTitle;
};

#endif // TITLEITERATOR_H
