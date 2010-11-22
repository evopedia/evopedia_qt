QT += core gui network
INCLUDEPATH += src

SOURCES +=  src/mainwindow.cpp \
        src/localarchive.cpp \
	src/title.cpp \
	src/titleiterator.cpp \
	src/titlelistmodel.cpp \
	src/evopedia.cpp \
	src/bzreader.cpp \
	src/evopediawebserver.cpp \
	src/utils.cpp \
        src/archive.cpp \
        src/downloadablearchive.cpp \
        src/partialarchive.cpp \
        src/map.cpp \
	src/flickable.cpp \
	src/dumpsettings.cpp \
	src/mapwindow.cpp \
	src/flickablemap.cpp \
	src/evopediaapplication.cpp \
	src/archivemanager.cpp \
        src/archivelist.cpp \
        src/tilefetcher.cpp \
        src/torrent/bencodeparser.cpp \
	src/torrent/connectionmanager.cpp \
	src/torrent/filemanager.cpp \
	src/torrent/metainfo.cpp \
	src/torrent/peerwireclient.cpp \
	src/torrent/ratecontroller.cpp \
	src/torrent/torrentclient.cpp \
	src/torrent/torrentserver.cpp \
	src/torrent/trackerclient.cpp

HEADERS += src/mainwindow.h \
        src/localarchive.h \
	src/title.h \
	src/titlelistmodel.h \
	src/titleiterator.h \
	src/evopedia.h \
	src/bzreader.h \
	src/evopediawebserver.h \
	src/utils.h \
        src/archive.h \
        src/downloadablearchive.h \
        src/partialarchive.h \
        src/map.h \
	src/geotitle.h \
	src/flickable.h \
	src/dumpsettings.h \
	src/mapwindow.h \
	src/flickablemap.h \
	src/evopediaapplication.h \
	src/defines.h \
	src/archivemanager.h \
        src/archivelist.h \
        src/tilefetcher.h \
	src/torrent/bencodeparser.h \
	src/torrent/connectionmanager.h \
	src/torrent/filemanager.h \
	src/torrent/metainfo.h \
	src/torrent/peerwireclient.h \
	src/torrent/ratecontroller.h \
	src/torrent/torrentclient.h \
	src/torrent/torrentserver.h \
	src/torrent/trackerclient.h

TRANSLATIONS += src/tr/evopedia_de.ts src/tr/evopedia_fr.ts src/tr/evopedia_es.ts src/tr/evopedia_en.ts src/tr/evopedia_nl.ts

FORMS += src/mainwindow.ui \
    src/dumpSettings.ui \
    src/mapwindow.ui

CONFIG += warn_on
maemo5 {
    CONFIG += mobility
    DEFINES += USE_MOBILITY
    MOBILITY += location
}
!symbian {
    LIBS += -lbz2
}
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_CAST_FROM_BYTEARRAY

OTHER_FILES += \
    src/evopedia.desktop \
    src/evopedia.js \
    src/header.html \
    src/wikipedia48.png \
    src/wikipedia.png \
    src/transtbl.dat \
    src/random.png \
    src/maparticle.png \
    src/map.png \
    src/main.css \
    src/magnify-clip.png \
    src/footer.html

RESOURCES += \
    src/resources.qrc \
    src/torrent/icons.qrc

unix {
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }
  BINDIR = $$PREFIX/bin
  DATADIR =$$PREFIX/share

  maemo5 {
      BINDIR = /opt/maemo/usr/bin
  }

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL

  INSTALLS += target desktop iconxpm

  target.path =$$BINDIR

  desktop.path = $$DATADIR/applications
  maemo5 {
      desktop.path = $$DATADIR/applications/hildon
  }
  desktop.files += src/$${TARGET}.desktop

  iconxpm.path = $$DATADIR/pixmaps
  iconxpm.files += src/evopedia-64x64.png
}
