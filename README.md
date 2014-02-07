# Evopedia

Offline Wikipedia Viewer

For more information, please visit the [website](http://evopedia.info).

## Compiling

Evopedia uses Qt and qmake, so simply running these commands on Ubuntu 12.04 (example):

    # apt-get install -y qt4-qmake libqt4-dev build-essential libghc-bzlib-dev
    # git clone https://github.com/evopedia/evopedia_qt.git
    # cd evopedia_qt
    # qmake
    # make
    # make install

should build Evopedia.

### Without QtGui

Evopedia can be compiled without dependency on QtGui, which is needed for some
platforms. Evopedia then does not start any GUI but can be controlled using
a web interface. To compile it without QtGui, use `qmake DEFINES+=NO_GUI`.

### Windows

For instructions on how to build Evopedia for Windows, please see the following
two articles.

 - [https://invalidmagic.wordpress.com/2010/12/25/evopedia-on-windows-with-installer/](https://invalidmagic.wordpress.com/2010/12/25/evopedia-on-windows-with-installer/)
 - [https://invalidmagic.wordpress.com/2010/10/20/evopedia-is-running-on-windows-xp/](https://invalidmagic.wordpress.com/2010/10/20/evopedia-is-running-on-windows-xp/)
