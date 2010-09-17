# Common definitions

QXMPP_INCLUDE_DIR = $$PWD/src
QXMPP_LIBRARY_DIR = $$PWD/lib

CONFIG(debug, debug|release) {
    QXMPP_LIBRARY_NAME = qxmpp_d
} else {
    QXMPP_LIBRARY_NAME = qxmpp
}

QXMPP_LIBS = -L$$QXMPP_LIBRARY_DIR -l$${QXMPP_LIBRARY_NAME}

# Symbian needs a .lib extension to recognise the library as static
symbian {
	QXMPP_LIBS = -L$$QXMPP_LIBRARY_DIR -l$${QXMPP_LIBRARY_NAME}.lib
}

# FIXME: we should be able to use the link_prl option to automatically pull
# in the extra libraries which the qxmpp library needs, but this does not
# seem to work on win32, so we specify the dependencies here:
unix {
    QXMPP_LIBS += -lresolv
}
win32 {
    QXMPP_LIBS += -ldnsapi
}
