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
    QXmppBind.h \
    QXmppClient.h \
    QXmppConfiguration.h \
    QXmppConstants.h \
    QXmppIq.h \
    QXmppMessage.h \
    QXmppPacket.h \
    QXmppPresence.h \
    QXmppRoster.h \
    QXmppRosterIq.h \
    QXmppSession.h \
    QXmppStanza.h \
    QXmppStream.h \
    QXmppLogger.h \
    QXmppReconnectionManager.h \
    QXmppVCardManager.h \
    QXmppVCard.h \
    QXmppNonSASLAuth.h \
    QXmppInformationRequestResult.h \
    QXmppDataIq.h \
    QXmppIbbIqs.h \
    QXmppIbbTransferJob.h \
    QXmppIbbTransferManager.h \
    xmlrpc.h \
    QXmppInvokable.h \
    QXmppRpcIq.h

# Source files
SOURCES += QXmppUtils.cpp \
    QXmppBind.cpp \
    QXmppClient.cpp \
    QXmppConfiguration.cpp \
    QXmppConstants.cpp \
    QXmppIq.cpp \
    QXmppMessage.cpp \
    QXmppPacket.cpp \
    QXmppPresence.cpp \
    QXmppRoster.cpp \
    QXmppRosterIq.cpp \
    QXmppSession.cpp \
    QXmppStanza.cpp \
    QXmppStream.cpp \
    QXmppLogger.cpp \
    QXmppReconnectionManager.cpp \
    QXmppVCardManager.cpp \
    QXmppVCard.cpp \
    QXmppNonSASLAuth.cpp \
    QXmppInformationRequestResult.cpp \
    QXmppDataIq.cpp \
    QXmppIbbIqs.cpp \
    QXmppIbbTransferJob.cpp \
    QXmppIbbTransferManager.cpp \
    xmlrpc.cpp \
    QXmppInvokable.cpp \
    QXmppRpcIq.cpp 
