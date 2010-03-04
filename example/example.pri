TEMPLATE = app

INCLUDEPATH += ../../source

QT += network xml

CONFIG += console debug_and_release

CONFIG(debug, debug|release) {
    QXMPP_LIB = QXmppClient_d
} else {
    QXMPP_LIB = QXmppClient
}
LIBS += -L../../source -l$$QXMPP_LIB

win32 {
    PRE_TARGETDEPS += ../../source/${QXMPP_LIB}.lib
} else {
    PRE_TARGETDEPS += ../../source/lib$${QXMPP_LIB}.a
}

