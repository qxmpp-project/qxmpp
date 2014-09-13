include(../qxmpp.pri)

QT -= gui
QT += testlib
CONFIG -= app_bundle
CONFIG += testcase

QMAKE_LIBDIR += ../../src
QMAKE_RPATHDIR += $$OUT_PWD/../../src
INCLUDEPATH += $$PWD $$QXMPP_INCLUDEPATH
LIBS += $$QXMPP_LIBS
