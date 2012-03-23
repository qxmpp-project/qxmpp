include(../qxmpp.pri)

QT += testlib

TARGET = qxmpp-tests

RESOURCES += tests.qrc
SOURCES += tests.cpp
HEADERS += tests.h 

INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += -L../src $$QXMPP_LIBS
