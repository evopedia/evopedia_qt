/*
 * evopedia: An offline Wikipedia reader.
 *
 * Copyright (C) 2010-2011 evopedia developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EVOPEDIAAPPLICATION_H
#define EVOPEDIAAPPLICATION_H

#if !defined(NO_GUI)
#include <QApplication>
#include <QDesktopServices>
#endif
#include <QCoreApplication>

#include "evopedia.h"
#if defined(NO_GUI)
class MainWindow {};
#else
#include "mainwindow.h"
#endif


class EvopediaApplication :
#if defined(NO_GUI)
        public QCoreApplication
#else
        public QApplication
#endif
{
    Q_OBJECT
public:
    explicit EvopediaApplication(int &argc, char **argv);
    ~EvopediaApplication();

    Evopedia *evopedia() { return m_evopedia; }

#if !defined(NO_GUI)
    void openArticle(const Title &title)
    {
        QDesktopServices::openUrl(m_evopedia->getArticleUrl(title));
    }
#endif


signals:

public slots:
private:
    Evopedia *m_evopedia;
    MainWindow *m_mainwindow;

};

#endif // EVOPEDIAAPPLICATION_H
