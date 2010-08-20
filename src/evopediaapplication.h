#ifndef EVOPEDIAAPPLICATION_H
#define EVOPEDIAAPPLICATION_H

#include <QApplication>
#include <QDesktopServices>

#include "evopedia.h"
#include "mainwindow.h"

class EvopediaApplication : public QApplication
{
    Q_OBJECT
public:
    explicit EvopediaApplication(int &argc, char **argv);
    ~EvopediaApplication();

    Evopedia *evopedia() { return m_evopedia; }

    void openArticle(const Title &title)
    {
        QDesktopServices::openUrl(m_evopedia->getArticleUrl(title));
    }


signals:

public slots:
private:
    Evopedia *m_evopedia;
    MainWindow *m_mainwindow;

};

#endif // EVOPEDIAAPPLICATION_H
