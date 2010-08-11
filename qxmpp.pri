CONFIG += debug_and_release

QXMPP_VERSION = 0.1.91
QXMPP_INCLUDE_DIR = $$PWD/source
CONFIG(debug, debug|release) {
    QXMPP_LIBRARY_DIR = $$PWD/source/debug
    QXMPP_LIBRARY_NAME = qxmpp_d
} else {
    QXMPP_LIBRARY_DIR = $$PWD/source/release
    QXMPP_LIBRARY_NAME = qxmpp
}

