#ifndef ABSTRACTFRONTEND_H
#define ABSTRACTFRONTEND_H

#include <QObject>

class QMenu;
class QString;
class QUrl;

class AbstractFrontend : public QObject {
Q_OBJECT
public:
    AbstractFrontend();
    virtual void saveSettings() = 0;
    virtual void unsaveSettings() = 0;
    virtual QMenu* createContextMenu() = 0;
    virtual bool validate(QString &ret) = 0;
    virtual QString language() = 0;
    virtual QString date() = 0;
    virtual QString archiveDir() = 0;
    virtual QString size() = 0;
    virtual QString stateString() = 0;
};

#endif // ABSTRACTFRONTEND_H
