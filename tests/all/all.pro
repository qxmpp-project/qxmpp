include(../tests.pri)

TARGET = tst_all

RESOURCES += tests.qrc
SOURCES += \
    dataform.cpp \
    jingle.cpp \
    register.cpp \
    roster.cpp \
    rpc.cpp \
    rsm.cpp \
    rtp.cpp \
    stanza.cpp \
    stun.cpp \
    tests.cpp
HEADERS += \
    dataform.h \
    jingle.h \
    register.h \
    roster.h \
    rpc.h \
    rsm.h \
    rtp.h \
    stanza.h \
    stun.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += codec.h sasl.h si.h
    SOURCES += codec.cpp sasl.cpp si.cpp
}
