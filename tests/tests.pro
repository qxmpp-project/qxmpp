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
    roster.cpp \
    rpc.cpp \
    rsm.cpp \
    rtp.cpp \
    si.cpp \
    stanza.cpp \
    stun.cpp \
    tests.cpp \
    vcard.cpp
HEADERS += \
    dataform.h \
    iq.h \
    jingle.h \
    message.h \
    presence.h \
    register.h \
    roster.h \
    rpc.h \
    rsm.h \
    rtp.h \
    si.h \
    stanza.h \
    stun.h \
    tests.h \
    vcard.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += codec.h sasl.h si.h
    SOURCES += codec.cpp sasl.cpp si.cpp
}

QMAKE_LIBDIR += ../src
INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += $$QXMPP_LIBS
