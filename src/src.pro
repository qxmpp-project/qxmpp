include(../qxmpp.pri)

QT -= gui

TEMPLATE = lib

CONFIG += staticlib
INCLUDEPATH += $$QXMPP_INCLUDEPATH $$QXMPP_INTERNAL_INCLUDES
LIBS += $$QXMPP_INTERNAL_LIBS

DEFINES += QT_STATICPLUGIN

# To enable support for the Speex audio codec, uncomment the following:
# DEFINES += QXMPP_USE_SPEEX
# LIBS += -lspeex

# To enable support for the Theora video codec, uncomment the following:
# DEFINES += QXMPP_USE_THEORA
# LIBS += -ltheoradec -ltheoraenc

# To enable support for the Vpx video codec, uncomment the following:
# DEFINES += QXMPP_USE_VPX
# LIBS += -lvpx

# Target definition
TARGET = $$QXMPP_LIBRARY_NAME
VERSION = $$QXMPP_VERSION
win32 {
    DESTDIR = $$OUT_PWD
}

include(base/base.pri)
include(client/client.pri)
include(server/server.pri)

HEADERS += $$INSTALL_HEADERS

# Installation
headers.files = $$INSTALL_HEADERS
headers.path = $$PREFIX/include/qxmpp
target.path = $$PREFIX/$$LIBDIR
INSTALLS += headers target

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
