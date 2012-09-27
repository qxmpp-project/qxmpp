include(../qxmpp.pri)

QT -= gui
QT += testlib

QMAKE_LIBDIR += ../../src
INCLUDEPATH += $$PWD $$QXMPP_INCLUDEPATH
LIBS += $$QXMPP_LIBS
