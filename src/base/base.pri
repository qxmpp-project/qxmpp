# Header files
INSTALL_HEADERS += \
    base/qdnslookup.h \
    base/qdnslookup_p.h \
    base/QXmppArchiveIq.h \
    base/QXmppBindIq.h \
    base/QXmppBookmarkSet.h \
    base/QXmppByteStreamIq.h \
    base/QXmppCodec.h \
    base/QXmppConstants.h \
    base/QXmppDataForm.h \
    base/QXmppDiscoveryIq.h \
    base/QXmppElement.h \
    base/QXmppEntityTimeIq.h \
    base/QXmppGlobal.h \
    base/QXmppIbbIq.h \
    base/QXmppIq.h \
    base/QXmppJingleIq.h \
    base/QXmppLogger.h \
    base/QXmppMessage.h \
    base/QXmppMucIq.h \
    base/QXmppNonSASLAuth.h \
    base/QXmppPingIq.h \
    base/QXmppPresence.h \
    base/QXmppPubSubIq.h \
    base/QXmppRosterIq.h \
    base/QXmppRpcIq.h \
    base/QXmppRtpChannel.h \
    base/QXmppSaslAuth.h \
    base/QXmppSessionIq.h \
    base/QXmppSocks.h \
    base/QXmppStanza.h \
    base/QXmppStream.h \
    base/QXmppStreamFeatures.h \
    base/QXmppStreamInitiationIq.h \
    base/QXmppStun.h \
    base/QXmppUtils.h \
    base/QXmppVCardIq.h \
    base/QXmppVersionIq.h

# Source files
SOURCES += \
    base/QXmppArchiveIq.cpp \
    base/QXmppBindIq.cpp \
    base/QXmppBookmarkSet.cpp \
    base/QXmppByteStreamIq.cpp \
    base/QXmppCodec.cpp \
    base/QXmppConstants.cpp \
    base/QXmppDataForm.cpp \
    base/QXmppDiscoveryIq.cpp \
    base/QXmppElement.cpp \
    base/QXmppEntityTimeIq.cpp \
    base/QXmppGlobal.cpp \
    base/QXmppIbbIq.cpp \
    base/QXmppIq.cpp \
    base/QXmppJingleIq.cpp \
    base/QXmppLogger.cpp \
    base/QXmppMessage.cpp \
    base/QXmppMucIq.cpp \
    base/QXmppNonSASLAuth.cpp \
    base/QXmppPingIq.cpp \
    base/QXmppPresence.cpp \
    base/QXmppPubSubIq.cpp \
    base/QXmppRosterIq.cpp \
    base/QXmppRpcIq.cpp \
    base/QXmppRtpChannel.cpp \
    base/QXmppSaslAuth.cpp \
    base/QXmppSessionIq.cpp \
    base/QXmppSocks.cpp \
    base/QXmppStanza.cpp \
    base/QXmppStream.cpp \
    base/QXmppStreamFeatures.cpp \
    base/QXmppStreamInitiationIq.cpp \
    base/QXmppStun.cpp \
    base/QXmppUtils.cpp \
    base/QXmppVCardIq.cpp \
    base/QXmppVersionIq.cpp

# DNS
SOURCES += base/qdnslookup.cpp
android:SOURCES += base/qdnslookup_stub.cpp
else:symbian:SOURCES += base/qdnslookup_symbian.cpp
else:unix:SOURCES += base/qdnslookup_unix.cpp
else:win32:SOURCES += base/qdnslookup_win.cpp
