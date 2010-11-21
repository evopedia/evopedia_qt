#ifndef UTILS_H
#define UTILS_H

#include <QtGlobal>
#include <QPair>
#include <QByteArray>

quint32 randomNumber(quint32 maxExcl);
QPair<qreal, qreal> parseCoordinatesInArticle(QByteArray &text, bool *error=0, int *zoom=0);
int parseCoordinatesZoom(const QString &zoomstr);
inline Qt::LayoutDirection getLayoutDirection(const QString &language)
{
    /* TODO1 are these all RTL languages of Wikipedia? */
    if (language == "ar" || language == "he" || language == "yi" || language == "ur" || language == "ckb") {
        return Qt::RightToLeft;
    } else {
        return Qt::LeftToRight;
    }
}

#endif // UTILS_H
