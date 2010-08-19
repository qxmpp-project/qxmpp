TEMPLATE = lib
QT += network \
    xml
CONFIG += staticlib \
    debug_and_release

INCLUDEPATH += ../
	
# Make sure the library gets built in the same location
# regardless of the platform. On win32 the library is
# automagically put in debug/release folders, so do the
# same for other platforms.
CONFIG(debug, debug|release) { 
    win32:TARGET = QXmppServer_d
    !win32:TARGET = debug/QXmppServer_d
} else {
    win32:TARGET = QXmppServer
    !win32:TARGET = release/QXmppServer
}

# Header files
HEADERS += QXmppClientServer.h \
    QXmppServer.h \
    QXmppServerConnection.h

# Source files
SOURCES += QXmppClientServer.cpp \
    QXmppServer.cpp \
    QXmppServerConnection.cpp

# Header files of the client
HEADERS += ../QXmppUtils.h \
    ../QXmppArchiveIq.h \
    ../QXmppArchiveManager.h \
    ../QXmppBind.h \
    ../QXmppByteStreamIq.h \
    ../QXmppClient.h \
    ../QXmppConfiguration.h \
    ../QXmppConstants.h \
    ../QXmppDataForm.h \
    ../QXmppDiscoveryIq.h \
    ../QXmppElement.h \
    ../QXmppIbbIq.h \
    ../QXmppInformationRequestResult.h \
    ../QXmppInvokable.h \
    ../QXmppIq.h \
    ../QXmppLogger.h \
    ../QXmppMessage.h \
    ../QXmppNonSASLAuth.h \
    ../QXmppPacket.h \
    ../QXmppPingIq.h \
    ../QXmppPresence.h \
    ../QXmppRoster.h \
    ../QXmppRosterIq.h \
    ../QXmppSession.h \
    ../QXmppSocks.h \
    ../QXmppStanza.h \
    ../QXmppStream.h \
    ../QXmppStreamInitiationIq.h \
    ../QXmppTransferManager.h \
    ../QXmppReconnectionManager.h \
    ../QXmppRemoteMethod.h \
    ../QXmppRpcIq.h \
    ../QXmppVCardManager.h \
    ../QXmppVCard.h \
    ../QXmppVersionIq.h \
    ../xmlrpc.h

# Source files of the client
SOURCES += ../QXmppUtils.cpp \
    ../QXmppArchiveIq.cpp \
    ../QXmppArchiveManager.cpp \
    ../QXmppBind.cpp \
    ../QXmppByteStreamIq.cpp \
    ../QXmppClient.cpp \
    ../QXmppConfiguration.cpp \
    ../QXmppConstants.cpp \
    ../QXmppDataForm.cpp \
    ../QXmppDiscoveryIq.cpp \
    ../QXmppElement.cpp \
    ../QXmppIbbIq.cpp \
    ../QXmppInformationRequestResult.cpp \
    ../QXmppInvokable.cpp \
    ../QXmppIq.cpp \
    ../QXmppLogger.cpp \
    ../QXmppMessage.cpp \
    ../QXmppNonSASLAuth.cpp \
    ../QXmppPacket.cpp \
    ../QXmppPingIq.cpp \
    ../QXmppPresence.cpp \
    ../QXmppRoster.cpp \
    ../QXmppRosterIq.cpp \
    ../QXmppSession.cpp \
    ../QXmppSocks.cpp \
    ../QXmppStanza.cpp \
    ../QXmppStream.cpp \
    ../QXmppStreamInitiationIq.cpp \
    ../QXmppTransferManager.cpp \
    ../QXmppReconnectionManager.cpp \
    ../QXmppRemoteMethod.cpp \
    ../QXmppRpcIq.cpp \
    ../QXmppVCardManager.cpp \
    ../QXmppVCard.cpp \
    ../QXmppVersionIq.cpp \
    ../xmlrpc.cpp

	
