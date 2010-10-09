#ifndef ABSTRACTFRONTEND_H
#define ABSTRACTFRONTEND_H

#include <QObject>

class QMenu;
class QString;

class AbstractFrontend : public QObject {
Q_OBJECT
public:
    AbstractFrontend();
    virtual void saveSettings() = 0;
    virtual void unsaveSettings() = 0;
    virtual QMenu* createContextMenu() = 0;
    virtual bool validate(QString &ret) = 0;
};

#endif // ABSTRACTFRONTEND_H
