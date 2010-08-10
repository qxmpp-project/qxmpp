CONFIG += debug_and_release

CONFIG(debug, debug|release) {
    QXMPP_LIB = qxmpp_d
} else {
    QXMPP_LIB = qxmpp
}
