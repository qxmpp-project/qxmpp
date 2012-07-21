include(../qxmpp.pri)

QT -= gui
QT += testlib

TARGET = qxmpp-tests

RESOURCES += tests.qrc
SOURCES += \
    dataform.cpp \
    iq.cpp \
    jingle.cpp \
    message.cpp \
    presence.cpp \
    register.cpp \
    rpc.cpp \
    rsm.cpp \
    rtp.cpp \
    stun.cpp \
    tests.cpp
HEADERS += \
    dataform.h \
    iq.h \
    jingle.h \
    message.h \
    presence.h \
    register.h \
    rpc.h \
    rsm.h \
    rtp.h \
    stun.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += codec.h sasl.h
    SOURCES += codec.cpp sasl.cpp
}

INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += -L../src $$QXMPP_LIBS
