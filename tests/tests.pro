include(../qxmpp.pri)

QT += testlib

TARGET = tests

RESOURCES += tests.qrc
SOURCES += tests.cpp
HEADERS += tests.h 

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += $$QXMPP_LIBS
PRE_TARGETDEPS += $$QXMPP_LIBRARY_FILE
