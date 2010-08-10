include(../qxmpp.pri)

TEMPLATE = app

INCLUDEPATH += ../../source

QT += network xml

CONFIG += console

CONFIG(debug, debug|release) {
    QXMPP_DIR = ../../source/debug
} else {
    QXMPP_DIR = ../../source/release
}

LIBS += -L$$QXMPP_DIR -l$$QXMPP_LIB
PRE_TARGETDEPS += $${QXMPP_DIR}/lib$${QXMPP_LIB}.a

