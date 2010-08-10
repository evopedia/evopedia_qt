QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT4 ) {
error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

symbian {
 TARGET.UID3 =  0xea2a762a
 TARGET.EPOCSTACKSIZE =  0x14000
 TARGET.EPOCHEAPSIZE =  0x020000  0x800000
}

TARGET = evopedia
TEMPLATE = app
include(src.pri)
symbian:include(bzlib.pri)
