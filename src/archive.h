#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QString>

class Archive
{
public:
    QString language;
    QString date;
    QString size;
    QString articleCount;

    /* TODO pointer to some bittorrent object? */

    enum State {Installed, Downloading, OnServer};

    State state;

    Archive();

    bool operator<(const Archive &other) const {
        if (language == other.language) {
            return date > other.date;
        } else {
            return language < other.language;
        }
    }
};

#endif // ARCHIVE_H
