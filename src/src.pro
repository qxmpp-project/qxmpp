include(../qxmpp.pri)

TEMPLATE = lib

QT += network xml

CONFIG += staticlib

# To disable the dependency on QtGui, uncomment the following:
# DEFINES += QXMPP_NO_GUI
# QT -= gui

# To enable support for the Speex codec, uncomment the following:
# DEFINES += QXMPP_USE_SPEEX
# LIBS += -lspeex

# DNS support
unix {
    LIBS += -lresolv
}
win32 {
    LIBS += -ldnsapi
}

# Target definition
TARGET = $$QXMPP_LIBRARY_NAME
VERSION = $$QXMPP_VERSION
DESTDIR = $$QXMPP_LIBRARY_DIR

# Header files
INSTALL_HEADERS = QXmppUtils.h \
    QXmppArchiveIq.h \
    QXmppArchiveManager.h \
    QXmppBindIq.h \
    QXmppByteStreamIq.h \
    QXmppCallManager.h \
    QXmppClient.h \
    QXmppClientExtension.h \
    QXmppCodec.h \
    QXmppConfiguration.h \
    QXmppConstants.h \
    QXmppDataForm.h \
    QXmppDialback.h \
    QXmppDiscoveryIq.h \
    QXmppElement.h \
    QXmppIbbIq.h \
    QXmppIncomingClient.h \
    QXmppIncomingServer.h \
    QXmppInvokable.h \
    QXmppIq.h \
    QXmppJingleIq.h \
    QXmppLogger.h \
    QXmppMessage.h \
    QXmppMucIq.h \
    QXmppMucManager.h \
    QXmppNonSASLAuth.h \
    QXmppOutgoingClient.h \
    QXmppOutgoingServer.h \
    QXmppPacket.h \
    QXmppPingIq.h \
    QXmppPresence.h \
    QXmppRoster.h \
    QXmppRosterIq.h \
    QXmppRosterManager.h \
    QXmppSaslAuth.h \
    QXmppServer.h \
    QXmppServerExtension.h \
    QXmppServerPlugin.h \
    QXmppSrvInfo.h \
    QXmppSessionIq.h \
    QXmppSocks.h \
    QXmppStanza.h \
    QXmppStream.h \
    QXmppStreamFeatures.h \
    QXmppStreamInitiationIq.h \
    QXmppStun.h \
    QXmppTransferManager.h \
    QXmppReconnectionManager.h \
    QXmppRemoteMethod.h \
    QXmppRpcIq.h \
    QXmppVCardManager.h \
    QXmppVCardIq.h \
    QXmppVCard.h \
    QXmppVersionIq.h\
    QXmppVersionManager.h \
    QXmppGlobal.h

HEADERS += $$INSTALL_HEADERS

# Source files
SOURCES += QXmppUtils.cpp \
    QXmppArchiveIq.cpp \
    QXmppArchiveManager.cpp \
    QXmppBindIq.cpp \
    QXmppByteStreamIq.cpp \
    QXmppCallManager.cpp \
    QXmppClient.cpp \
    QXmppClientExtension.cpp \
    QXmppCodec.cpp \
    QXmppConfiguration.cpp \
    QXmppConstants.cpp \
    QXmppDataForm.cpp \
    QXmppDialback.cpp \
    QXmppDiscoveryIq.cpp \
    QXmppElement.cpp \
    QXmppIbbIq.cpp \
    QXmppIncomingClient.cpp \
    QXmppIncomingServer.cpp \
    QXmppInvokable.cpp \
    QXmppIq.cpp \
    QXmppJingleIq.cpp \
    QXmppLogger.cpp \
    QXmppMessage.cpp \
    QXmppMucIq.cpp \
    QXmppMucManager.cpp \
    QXmppNonSASLAuth.cpp \
    QXmppOutgoingClient.cpp \
    QXmppOutgoingServer.cpp \
    QXmppPacket.cpp \
    QXmppPingIq.cpp \
    QXmppPresence.cpp \
    QXmppRosterIq.cpp \
    QXmppRosterManager.cpp \
    QXmppSaslAuth.cpp \
    QXmppServer.cpp \
    QXmppServerExtension.cpp \
    QXmppSrvInfo.cpp \
    QXmppSessionIq.cpp \
    QXmppSocks.cpp \
    QXmppStanza.cpp \
    QXmppStream.cpp \
    QXmppStreamFeatures.cpp \
    QXmppStreamInitiationIq.cpp \
    QXmppStun.cpp \
    QXmppTransferManager.cpp \
    QXmppReconnectionManager.cpp \
    QXmppRemoteMethod.cpp \
    QXmppRpcIq.cpp \
    QXmppVCardManager.cpp \
    QXmppVCardIq.cpp \
    QXmppVersionIq.cpp \
    QXmppVersionManager.cpp \
    QXmppGlobal.cpp


# Plugins
DEFINES += QT_STATICPLUGIN
HEADERS += \
    server/mod_disco.h \
    server/mod_ping.h \
    server/mod_proxy65.h \
    server/mod_stats.h \
    server/mod_version.h
SOURCES += \
    server/mod_disco.cpp \
    server/mod_ping.cpp \
    server/mod_proxy65.cpp \
    server/mod_stats.cpp \
    server/mod_version.cpp

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_DESTDIR = $$QXMPP_LIBRARY_DIR/pkgconfig

# Installation
headers.files = $$INSTALL_HEADERS
headers.path = $$[QT_INSTALL_PREFIX]/include/qxmpp
target.path = $$[QT_INSTALL_PREFIX]/lib
INSTALLS += headers target
