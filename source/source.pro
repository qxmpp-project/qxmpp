TEMPLATE = lib
QT += network \
    xml
CONFIG += staticlib \
    debug_and_release
CONFIG(debug, debug|release) { 
    win32:TARGET = QXmppClient_d
    !win32:TARGET = debug/QXmppClient_d
}
else { 
    win32:TARGET = QXmppClient
    !win32:TARGET = release/QXmppClient
}

# Header files
HEADERS += QXmppUtils.h \
    QXmppArchiveIq.h \
    QXmppArchiveManager.h \
    QXmppBind.h \
    QXmppClient.h \
    QXmppConfiguration.h \
    QXmppConstants.h \
    QXmppDiscoveryIq.h \
    QXmppElement.h \
    QXmppIq.h \
    QXmppMessage.h \
    QXmppPacket.h \
    QXmppPingIq.h \
    QXmppPresence.h \
    QXmppRoster.h \
    QXmppRosterIq.h \
    QXmppSession.h \
    QXmppStanza.h \
    QXmppStream.h \
    QXmppStreamInitiationIq.h \
    QXmppTransferManager.h \
    QXmppLogger.h \
    QXmppReconnectionManager.h \
    QXmppVCardManager.h \
    QXmppVCard.h \
    QXmppNonSASLAuth.h \
    QXmppInformationRequestResult.h \
    QXmppIbbIqs.h \
    xmlrpc.h \
    QXmppInvokable.h \
    QXmppRpcIq.h \
    QXmppRemoteMethod.h

# Source files
SOURCES += QXmppUtils.cpp \
    QXmppArchiveIq.cpp \
    QXmppArchiveManager.cpp \
    QXmppBind.cpp \
    QXmppClient.cpp \
    QXmppConfiguration.cpp \
    QXmppConstants.cpp \
    QXmppDiscoveryIq.cpp \
    QXmppElement.cpp \
    QXmppIq.cpp \
    QXmppMessage.cpp \
    QXmppPacket.cpp \
    QXmppPingIq.cpp \
    QXmppPresence.cpp \
    QXmppRoster.cpp \
    QXmppRosterIq.cpp \
    QXmppSession.cpp \
    QXmppStanza.cpp \
    QXmppStream.cpp \
    QXmppStreamInitiationIq.cpp \
    QXmppTransferManager.cpp \
    QXmppLogger.cpp \
    QXmppReconnectionManager.cpp \
    QXmppVCardManager.cpp \
    QXmppVCard.cpp \
    QXmppNonSASLAuth.cpp \
    QXmppInformationRequestResult.cpp \
    QXmppIbbIqs.cpp \
    xmlrpc.cpp \
    QXmppInvokable.cpp \
    QXmppRpcIq.cpp \
    QXmppRemoteMethod.cpp
