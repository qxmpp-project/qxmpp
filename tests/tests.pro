include(../qxmpp.pri)

QT += testlib

TARGET = qxmpp-tests

RESOURCES += tests.qrc
SOURCES += \
    dataform.cpp \
    jingle.cpp \
    message.cpp \
    presence.cpp \
    register.cpp \
    rsm.cpp \
    rtp.cpp \
    tests.cpp
HEADERS += \
    dataform.h \
    jingle.h \
    message.h \
    presence.h \
    register.h \
    rsm.h \
    rtp.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += codec.h sasl.h
    SOURCES += codec.cpp sasl.cpp
}

INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += -L../src $$QXMPP_LIBS
