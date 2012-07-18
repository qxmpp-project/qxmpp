include(../qxmpp.pri)

QT += testlib

TARGET = qxmpp-tests

RESOURCES += tests.qrc
SOURCES += \
    dataform.cpp \
    register.cpp \
    rtp.cpp \
    tests.cpp
HEADERS += \
    dataform.h \
    register.h \
    rtp.h \
    tests.h

INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += -L../src $$QXMPP_LIBS
