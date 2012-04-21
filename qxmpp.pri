# Common definitions

QT += network xml
QXMPP_VERSION = 0.4.0
QXMPP_INCLUDEPATH = $$PWD/src/base $$PWD/src/client $$PWD/src/server

# Determine library name
CONFIG(debug, debug|release) {
    QXMPP_LIBRARY_NAME = qxmpp_d
} else {
    QXMPP_LIBRARY_NAME = qxmpp
}

# Determine library type (lib or staticlib)
isEmpty(QXMPP_LIBRARY_TYPE) {
    QXMPP_LIBRARY_TYPE = staticlib
}

# Libraries used internally by QXmpp
android {

} else:contains(MEEGO_EDITION,harmattan) {
    # meego/harmattan has speex for sure
    QXMPP_INTERNAL_DEFINES += QXMPP_USE_SPEEX
    QXMPP_INTERNAL_LIBS += -lspeex
} else:symbian {
    QXMPP_INTERNAL_INCLUDES = $$APP_LAYER_SYSTEMINCLUDE
    QXMPP_INTERNAL_LIBS = -lesock
} else:win32 {
    QXMPP_INTERNAL_LIBS = -ldnsapi -lws2_32
}

# Libraries for apps which use QXmpp
QXMPP_LIBS = -l$${QXMPP_LIBRARY_NAME}
contains(QXMPP_LIBRARY_TYPE,staticlib) {
    # Symbian needs a .lib extension to recognise the library as static
    symbian: QXMPP_LIBS = -l$${QXMPP_LIBRARY_NAME}.lib

    # FIXME: we should be able to use the link_prl option to automatically pull
    # in the extra libraries which the qxmpp library needs, but this does not
    # seem to work on win32, so we specify the dependencies here:
    QXMPP_LIBS += $$QXMPP_INTERNAL_LIBS
    DEFINES += QXMPP_STATIC
} else {
    # Windows needs the major library version
    win32: QXMPP_LIBS = -l$${QXMPP_LIBRARY_NAME}0
    DEFINES += QXMPP_SHARED
}

# Installation prefix and library directory
isEmpty(PREFIX) {
    unix:PREFIX=/usr/local
}
isEmpty(LIBDIR) {
    LIBDIR=lib
}
