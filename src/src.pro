include(../qxmpp.pri)

QT -= gui

TEMPLATE = lib

CONFIG += staticlib
INCLUDEPATH += $$QXMPP_INCLUDEPATH $$QXMPP_INTERNAL_INCLUDES
LIBS += $$QXMPP_INTERNAL_LIBS

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
    QXmppVersionIq.h \
    client/QXmppArchiveManager.h \
    client/QXmppBookmarkManager.h \
    client/QXmppCallManager.h \
    client/QXmppClient.h \
    client/QXmppClientExtension.h \
    client/QXmppConfiguration.h \
    client/QXmppDiscoveryManager.h \
    client/QXmppEntityTimeManager.h \
    client/QXmppInvokable.h \
    client/QXmppMessageReceiptManager.h \
    client/QXmppMucManager.h \
    client/QXmppOutgoingClient.h \
    client/QXmppReconnectionManager.h \
    client/QXmppRemoteMethod.h \
    client/QXmppRosterManager.h \
    client/QXmppRpcManager.h \
    client/QXmppTransferManager.h \
    client/QXmppVCardManager.h \
    client/QXmppVersionManager.h \
    server/QXmppDialback.h \
    server/QXmppIncomingClient.h \
    server/QXmppIncomingServer.h \
    server/QXmppOutgoingServer.h \
    server/QXmppPasswordChecker.h \
    server/QXmppServer.h \
    server/QXmppServerExtension.h \
    server/QXmppServerPlugin.h

HEADERS += $$INSTALL_HEADERS

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
    QXmppVersionIq.cpp \
    client/QXmppDiscoveryManager.cpp \
    client/QXmppArchiveManager.cpp \
    client/QXmppBookmarkManager.cpp \
    client/QXmppCallManager.cpp \
    client/QXmppClient.cpp \
    client/QXmppClientExtension.cpp \
    client/QXmppConfiguration.cpp \
    client/QXmppEntityTimeManager.cpp \
    client/QXmppInvokable.cpp \
    client/QXmppMessageReceiptManager.cpp \
    client/QXmppMucManager.cpp \
    client/QXmppOutgoingClient.cpp \
    client/QXmppReconnectionManager.cpp \
    client/QXmppRemoteMethod.cpp \
    client/QXmppRosterManager.cpp \
    client/QXmppRpcManager.cpp \
    client/QXmppTransferManager.cpp \
    client/QXmppVCardManager.cpp \
    client/QXmppVersionManager.cpp \
    server/QXmppDialback.cpp \
    server/QXmppIncomingClient.cpp \
    server/QXmppIncomingServer.cpp \
    server/QXmppOutgoingServer.cpp \
    server/QXmppPasswordChecker.cpp \
    server/QXmppServer.cpp \
    server/QXmppServerExtension.cpp

# DNS
HEADERS += qdnslookup.h qdnslookup_p.h
SOURCES += qdnslookup.cpp
android:SOURCES += qdnslookup_stub.cpp
else:symbian:SOURCES += qdnslookup_symbian.cpp
else:unix:SOURCES += qdnslookup_unix.cpp
else:win32:SOURCES += qdnslookup_win.cpp

# Plugins
DEFINES += QT_STATICPLUGIN
HEADERS += \
    server/mod_disco.h \
    server/mod_ping.h \
    server/mod_presence.h \
    server/mod_proxy65.h \
    server/mod_stats.h \
    server/mod_time.h \
    server/mod_version.h
SOURCES += \
    server/mod_disco.cpp \
    server/mod_ping.cpp \
    server/mod_presence.cpp \
    server/mod_proxy65.cpp \
    server/mod_stats.cpp \
    server/mod_time.cpp \
    server/mod_version.cpp

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
