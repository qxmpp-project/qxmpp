include(../tests.pri)

TARGET = tst_all

RESOURCES += tests.qrc
SOURCES += \
    jingle.cpp \
    register.cpp \
    roster.cpp \
    rpc.cpp \
    rsm.cpp \
    stun.cpp \
    tests.cpp
HEADERS += \
    jingle.h \
    register.h \
    roster.h \
    rpc.h \
    rsm.h \
    stun.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += codec.h sasl.h si.h
    SOURCES += codec.cpp sasl.cpp si.cpp
}
