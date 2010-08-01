DEFINES += "EVOPEDIA_VERSION=\"\\\"0.4.0\\\"\""
DEFINES += "EVOPEDIA_WEBSITE=\"\\\"http://evopedia.info\\\"\""
DEFINES += "EVOPEDIA_DUMP_SITE=\"\\\"http://dumpathome.evopedia.info/dumps/finished\\\"\""
DEFINES += "EVOPEDIA_BUG_SITE=\"\\\"https://bugs.maemo.org/enter_bug.cgi?product=evopedia\\\"\""

QT += core gui network
TARGET = evopedia
TEMPLATE = app
SOURCES += main.cpp \
 mainwindow.cpp \
 storagebackend.cpp \
 title.cpp \
 titleiterator.cpp \
 titlelistmodel.cpp \
    evopedia.cpp \
    bzreader.cpp \
    evopediawebserver.cpp \
    utils.cpp \
    map.cpp \
    flickable.cpp \
    dumpsettings.cpp \
    mapwindow.cpp \
    flickablemap.cpp
HEADERS += mainwindow.h \
 storagebackend.h \
 title.h \
 titlelistmodel.h \
 titleiterator.h \
    evopedia.h \
    bzreader.h \
    evopediawebserver.h \
    utils.h \
    map.h \
    geotitle.h \
    flickable.h \
    dumpsettings.h \
    mapwindow.h \
    flickablemap.h

CONFIG += warn_on
LIBS += -lbz2
MOBILITY =
symbian {
 TARGET.UID3 =  0xea2a762a
 TARGET.EPOCSTACKSIZE =  0x14000
 TARGET.EPOCHEAPSIZE =  0x020000  0x800000
}
FORMS += mainwindow.ui \
    dumpSettings.ui \
    mapwindow.ui
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_CAST_FROM_BYTEARRAY

OTHER_FILES += \
    evopedia.desktop \
    evopedia.js \
    header.html \
    wikipedia48.png \
    wikipedia.png \
    transtbl.dat \
    random.png \
    maparticle.png \
    map.png \
    main.css \
    magnify-clip.png \
    footer.html

RESOURCES += \
    resources.qrc

unix {
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }
  BINDIR = $$PREFIX/bin
  DATADIR =$$PREFIX/share

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL

  INSTALLS += target desktop service iconxpm

  target.path =$$BINDIR

  desktop.path = $$DATADIR/applications/hildon
  desktop.files += $${TARGET}.desktop

  iconxpm.path = $$DATADIR/pixmaps
  iconxpm.files += wikipedia48.png
}
