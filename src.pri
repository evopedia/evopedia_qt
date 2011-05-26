QT += core gui network
INCLUDEPATH += src

DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_CAST_FROM_BYTEARRAY

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
	src/torrent/trackerclient.cpp \
        src/archivedetailsdialog.cpp

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
	src/torrent/trackerclient.h \
        src/archivedetailsdialog.h

TRANSLATIONS += \
        resources/tr/evopedia_ca.ts \
        resources/tr/evopedia_cz.ts \
        resources/tr/evopedia_de.ts \
        resources/tr/evopedia_en.ts \
        resources/tr/evopedia_es.ts \
        resources/tr/evopedia_fr.ts \
        resources/tr/evopedia_it.ts \
        resources/tr/evopedia_ja.ts \
        resources/tr/evopedia_nl.ts \
        resources/tr/evopedia_vi.ts

FORMS += src/ui/mainwindow.ui \
        src/ui/dumpSettings.ui \
        src/ui/mapwindow.ui \
        src/ui/archivedetailsdialog.ui

OTHER_FILES += \
        resources/evopedia.desktop \
        resources/evopedia.js \
        resources/header.html \
        resources/wikipedia48.png \
        resources/wikipedia.png \
        resources/transtbl.dat \
        resources/random.png \
        resources/maparticle.png \
        resources/map.png \
        resources/main.css \
        resources/magnify-clip.png \
        resources/footer.html

RESOURCES += \
        resources/resources.qrc \
        src/torrent/icons.qrc

CONFIG += warn_on

maemo5 {
        CONFIG += mobility
        DEFINES += USE_MOBILITY
        MOBILITY += location
}

windows {
    # download the lib/dll/include for bzip2 and copy it to the source directory
    INCLUDEPATH += bzip2/include
    LIBS += $$quote($$_PRO_FILE_PWD_)/bzip2/lib/bzip2.lib

    RC_FILE += \
       resources/windows/windows.rc
}

macx {
     ICON = resources/evopedia.icns
}

symbian {
     ICON = resources/evopedia.svg
}

unix {
  LIBS += -lbz2
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr
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
  desktop.files += resources/$${TARGET}.desktop

  iconxpm.path = $$DATADIR/pixmaps
  iconxpm.files += resources/evopedia-64x64.png
}
