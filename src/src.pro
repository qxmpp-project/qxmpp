include(../qxmpp.pri)

TEMPLATE = lib

QT += network xml

CONFIG += staticlib

# To disable the dependency on QtGui, uncomment the following:
# DEFINES += QXMPP_NO_GUI

# To enable support for the Speex codec, uncomment the following:
# DEFINES += QXMPP_USE_SPEEX

# Target definition
TARGET = $$QXMPP_LIBRARY_NAME
VERSION = $$QXMPP_VERSION
DESTDIR = $$QXMPP_LIBRARY_DIR

# Header files
HEADERS += QXmppUtils.h \
    QXmppArchiveIq.h \
    QXmppArchiveManager.h \
    QXmppBindIq.h \
    QXmppByteStreamIq.h \
    QXmppCallManager.h \
    QXmppClient.h \
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
    QXmppVCard.h \
    QXmppVersionIq.h

# Source files
SOURCES += QXmppUtils.cpp \
    QXmppArchiveIq.cpp \
    QXmppArchiveManager.cpp \
    QXmppBindIq.cpp \
    QXmppByteStreamIq.cpp \
    QXmppCallManager.cpp \
    QXmppClient.cpp \
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
    QXmppVCard.cpp \
    QXmppVersionIq.cpp

# pkg-config support
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_DESTDIR = $$QXMPP_LIBRARY_DIR/pkgconfig

# Installation
headers.files = $$HEADERS
headers.path = $$[QT_INSTALL_PREFIX]/include/qxmpp
target.path = $$[QT_INSTALL_PREFIX]/lib
INSTALLS += headers target
