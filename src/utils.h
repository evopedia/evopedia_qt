#ifndef UTILS_H
#define UTILS_H

#include <QtGlobal>
#include <QPair>
#include <QByteArray>

quint32 randomNumber(quint32 maxExcl);
QPair<qreal, qreal> parseCoordinatesInArticle(QByteArray &text, bool *error=0, int *zoom=0);
int parseCoordinatesZoom(const QString &zoomstr);
bool internetConnectionActive();


#endif // UTILS_H
