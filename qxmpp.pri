# Common definitions

QXMPP_VERSION = 0.1.91
QXMPP_INCLUDE_DIR = $$PWD/source
QXMPP_LIBRARY_DIR = $$PWD/lib

CONFIG(debug, debug|release) {
    QXMPP_LIBRARY_NAME = qxmpp_d
} else {
    QXMPP_LIBRARY_NAME = qxmpp
}

