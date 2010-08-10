include(../qxmpp.pri)

QT += network xml testlib

TARGET = tests

SOURCES += tests.cpp

HEADERS += tests.h 

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += -L$$QXMPP_LIBRARY_DIR -l$$QXMPP_LIB
