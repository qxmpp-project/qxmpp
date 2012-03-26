# Headers
INSTALL_HEADERS += \
    server/mod_presence.h \
    server/QXmppDialback.h \
    server/QXmppIncomingClient.h \
    server/QXmppIncomingServer.h \
    server/QXmppOutgoingServer.h \
    server/QXmppPasswordChecker.h \
    server/QXmppServer.h \
    server/QXmppServerExtension.h \
    server/QXmppServerPlugin.h

# Source files
SOURCES += \
    server/QXmppDialback.cpp \
    server/QXmppIncomingClient.cpp \
    server/QXmppIncomingServer.cpp \
    server/QXmppOutgoingServer.cpp \
    server/QXmppPasswordChecker.cpp \
    server/QXmppServer.cpp \
    server/QXmppServerExtension.cpp

# Plugins
HEADERS += \
    server/mod_disco.h \
    server/mod_time.h \
    server/mod_version.h
SOURCES += \
    server/mod_disco.cpp \
    server/mod_presence.cpp \
    server/mod_time.cpp \
    server/mod_version.cpp
