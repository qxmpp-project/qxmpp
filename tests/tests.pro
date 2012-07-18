include(../qxmpp.pri)

QT += testlib

TARGET = qxmpp-tests

RESOURCES += tests.qrc
SOURCES += \
    dataform.cpp \
    message.cpp \
    presence.cpp \
    register.cpp \
    rsm.cpp \
    rtp.cpp \
    tests.cpp
HEADERS += \
    dataform.h \
    message.h \
    presence.h \
    register.h \
    rsm.h \
    rtp.h \
    tests.h

INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += -L../src $$QXMPP_LIBS
