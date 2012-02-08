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
DESTDIR = $$QXMPP_LIBRARY_DIR

# Header files
INSTALL_HEADERS = \
    QXmppArchiveIq.h \
    QXmppBindIq.h \
    QXmppBookmarkSet.h \
    QXmppByteStreamIq.h \
    QXmppCodec.h \
    QXmppConstants.h \
    QXmppDataForm.h \
    QXmppDiscoveryIq.h \
    QXmppElement.h \
    QXmppEntityTimeIq.h \
    QXmppGlobal.h \
    QXmppIbbIq.h \
    QXmppIq.h \
    QXmppJingleIq.h \
    QXmppLogger.h \
    QXmppMessage.h \
    QXmppMucIq.h \
    QXmppNonSASLAuth.h \
    QXmppPacket.h \
    QXmppPingIq.h \
    QXmppPresence.h \
    QXmppPubSubIq.h \
    QXmppRosterIq.h \
    QXmppRpcIq.h \
    QXmppRtpChannel.h \
    QXmppSaslAuth.h \
    QXmppSessionIq.h \
    QXmppSocks.h \
    QXmppStanza.h \
    QXmppStream.h \
    QXmppStreamFeatures.h \
    QXmppStreamInitiationIq.h \
    QXmppStun.h \
    QXmppUtils.h \
    QXmppVCardIq.h \
    QXmppVersionIq.h

# Source files
SOURCES += \
    QXmppArchiveIq.cpp \
    QXmppBindIq.cpp \
    QXmppBookmarkSet.cpp \
    QXmppByteStreamIq.cpp \
    QXmppCodec.cpp \
    QXmppConstants.cpp \
    QXmppDataForm.cpp \
    QXmppDiscoveryIq.cpp \
    QXmppElement.cpp \
    QXmppEntityTimeIq.cpp \
    QXmppGlobal.cpp \
    QXmppIbbIq.cpp \
    QXmppIq.cpp \
    QXmppJingleIq.cpp \
    QXmppLogger.cpp \
    QXmppMessage.cpp \
    QXmppMucIq.cpp \
    QXmppNonSASLAuth.cpp \
    QXmppPacket.cpp \
    QXmppPingIq.cpp \
    QXmppPresence.cpp \
    QXmppPubSubIq.cpp \
    QXmppRosterIq.cpp \
    QXmppRpcIq.cpp \
    QXmppRtpChannel.cpp \
    QXmppSaslAuth.cpp \
    QXmppSessionIq.cpp \
    QXmppSocks.cpp \
    QXmppStanza.cpp \
    QXmppStream.cpp \
    QXmppStreamFeatures.cpp \
    QXmppStreamInitiationIq.cpp \
    QXmppStun.cpp \
    QXmppUtils.cpp \
    QXmppVCardIq.cpp \
    QXmppVersionIq.cpp

# DNS
HEADERS += qdnslookup.h qdnslookup_p.h
SOURCES += qdnslookup.cpp
android:SOURCES += qdnslookup_stub.cpp
else:symbian:SOURCES += qdnslookup_symbian.cpp
else:unix:SOURCES += qdnslookup_unix.cpp
else:win32:SOURCES += qdnslookup_win.cpp

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
QMAKE_PKGCONFIG_DESTDIR = $$QXMPP_LIBRARY_DIR/pkgconfig
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
