# Common definitions

QXMPP_INCLUDE_DIR = $$PWD/src
QXMPP_LIBRARY_DIR = $$PWD/lib

CONFIG(debug, debug|release) {
    QXMPP_LIBRARY_NAME = qxmpp_d
} else {
    QXMPP_LIBRARY_NAME = qxmpp
}

# Libraries used internal by QXmpp
symbian {
    QXMPP_INTERNAL_LIBS = -lesock
} else:unix {
    QXMPP_INTERNAL_LIBS = -lresolv
} else:win32 {
    QXMPP_INTERNAL_LIBS = -ldnsapi
}

# Libraries for apps which use QXmpp
symbian {
    # Symbian needs a .lib extension to recognise the library as static
    QXMPP_LIBS = -L$$QXMPP_LIBRARY_DIR -l$${QXMPP_LIBRARY_NAME}.lib
} else {
    QXMPP_LIBS = -L$$QXMPP_LIBRARY_DIR -l$${QXMPP_LIBRARY_NAME}
}

# FIXME: we should be able to use the link_prl option to automatically pull
# in the extra libraries which the qxmpp library needs, but this does not
# seem to work on win32, so we specify the dependencies here:
QXMPP_LIBS += $$QXMPP_INTERNAL_LIBS

# Path of the QXmpp library file, for expressing dependency.
symbian {
} else:unix {
    QXMPP_LIBRARY_FILE = $${QXMPP_LIBRARY_DIR}/lib$${QXMPP_LIBRARY_NAME}.a
}

