include(../qxmpp.pri)

QT += network xml testlib

TARGET = tests

RESOURCES += tests.qrc
SOURCES += tests.cpp
HEADERS += tests.h 

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += -L$$QXMPP_LIBRARY_DIR -l$$QXMPP_LIBRARY_NAME
