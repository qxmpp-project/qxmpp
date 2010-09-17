include(../qxmpp.pri)

QT += network xml testlib

TARGET = tests

RESOURCES += tests.qrc
SOURCES += tests.cpp
HEADERS += tests.h 

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += $$QXMPP_LIBS
