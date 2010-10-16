#include "titleiterator.h"

#include <QByteArray>

#include "localarchive.h"

TitleIterator::TitleIterator()
    : device(0), prefix(QString())
{
    checkHasNext();
}

TitleIterator::TitleIterator(QIODevice *device_ini, const QString &prefix, const QString &language)
    : language(language), device(device_ini), prefix(LocalArchive::normalize(prefix))
{
    checkHasNext();
}

bool TitleIterator::hasNext() const
{
    return !nextTitle.getName().isNull();
}

void TitleIterator::checkHasNext()
{
    if (device == 0 || device->atEnd()) {
        nextTitle = Title();
    } else {
        QByteArray line = device->readLine();
        nextTitle = Title(line.left(line.length() - 1), language);
        if (!prefix.isNull()) {
            QString tn = LocalArchive::normalize(nextTitle.getName());
            if (!tn.startsWith(prefix))
                nextTitle = Title();
        }
    }
}

const Title TitleIterator::next()
{
    Title t = nextTitle;
    checkHasNext();
    return t;
}
