#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QObject>
#include <QPair>
#include <QString>
#include <QSettings>

/* (language, date) uniquely identifies archive */
typedef QPair<QString, QString> ArchiveID;

class Archive : public QObject
{
    Q_OBJECT

protected:
    QString language;
    QString date;
public:
    explicit Archive(QObject *parent = 0);

    const QString &getLanguage() const { return language; }
    const QString &getDate() const { return date; }
    const ArchiveID getID() const { return ArchiveID(language, date); }

    virtual void saveToSettings(QSettings &settings) const { Q_UNUSED(settings); }

    /* newer archives are "less" */
    bool operator<(const Archive &other) const {
        if (getLanguage() == other.getLanguage())
            return getDate() > other.getDate();
        else
            return getLanguage() < other.getLanguage();
    }

    static bool comparePointers(const Archive *a, const Archive *b) {
        return (*a) < (*b);
    }

signals:

public slots:


};

#endif // ARCHIVE_H
