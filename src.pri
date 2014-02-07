QT += core network

!contains(DEFINES, NO_GUI) {
    QT += gui
}
INCLUDEPATH += src

DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += QT_NO_CAST_FROM_BYTEARRAY

SOURCES += \
    src/archive.cpp \
    src/archivemanager.cpp \
    src/bzreader.cpp \
    src/downloadablearchive.cpp \
    src/evopedia.cpp \
    src/evopediaapplication.cpp \
    src/evopediawebserver.cpp \
    src/localarchive.cpp \
    src/partialarchive.cpp \
    src/title.cpp \
    src/titleiterator.cpp \
    src/utils.cpp \
    src/torrent/bencodeparser.cpp \
    src/torrent/connectionmanager.cpp \
    src/torrent/filemanager.cpp \
    src/torrent/metainfo.cpp \
    src/torrent/peerwireclient.cpp \
    src/torrent/ratecontroller.cpp \
    src/torrent/torrentclient.cpp \
    src/torrent/torrentserver.cpp \
    src/torrent/trackerclient.cpp

HEADERS += \
    src/localarchive.h \
    src/title.h \
    src/evopedia.h \
    src/bzreader.h \
    src/evopediawebserver.h \
    src/utils.h \
    src/archive.h \
    src/downloadablearchive.h \
    src/partialarchive.h \
    src/titleiterator.h \
    src/evopediaapplication.h \
    src/defines.h \
    src/archivemanager.h \
    src/torrent/bencodeparser.h \
    src/torrent/connectionmanager.h \
    src/torrent/filemanager.h \
    src/torrent/metainfo.h \
    src/torrent/peerwireclient.h \
    src/torrent/ratecontroller.h \
    src/torrent/torrentclient.h \
    src/torrent/torrentserver.h \
    src/torrent/trackerclient.h

!contains(DEFINES, NO_GUI) {
    SOURCES += \
        src/archivedetailsdialog.cpp \
        src/archivelist.cpp \
        src/dumpsettings.cpp \
        src/flickable.cpp \
        src/flickablemap.cpp \
        src/mainwindow.cpp \
        src/map.cpp \
        src/mapwindow.cpp \
        src/tilefetcher.cpp \
        src/titlelistmodel.cpp
    
    HEADERS += \
        src/archivedetailsdialog.h \
        src/archivelist.h \
        src/dumpsettings.h \
        src/flickable.h \
        src/flickablemap.h \
        src/geotitle.h \
        src/mainwindow.h \
        src/map.h \
        src/mapwindow.h \
        src/tilefetcher.h \
        src/titlelistmodel.h

    FORMS += src/ui/mainwindow.ui \
        src/ui/dumpSettings.ui \
        src/ui/mapwindow.ui \
        src/ui/archivedetailsdialog.ui
}

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

OTHER_FILES += \
    resources/evopedia.desktop \
    resources/evopedia.js \
    resources/footer.html \
    resources/header.html \
    resources/magnify-clip.png \
    resources/main.css \
    resources/map.png \
    resources/maparticle.png \
    resources/random.png \
    resources/transtbl.dat \
    resources/wikipedia.png \
    resources/wikipedia48.png

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
