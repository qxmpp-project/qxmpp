TEMPLATE = app

INCLUDEPATH += ../../source

QT += network xml

CONFIG += console debug_and_release

CONFIG(debug, debug|release) {
    QXMPP_LIB = qxmpp_d
    QXMPP_DIR = ../../source/debug
} else {
    QXMPP_LIB = qxmpp
    QXMPP_DIR = ../../source/release
}

LIBS += -L$$QXMPP_DIR -l$$QXMPP_LIB
PRE_TARGETDEPS += $${QXMPP_DIR}/lib$${QXMPP_LIB}.a

