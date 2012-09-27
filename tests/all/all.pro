include(../tests.pri)

TARGET = tst_all

RESOURCES += tests.qrc
SOURCES += \
    jingle.cpp \
    rpc.cpp \
    stun.cpp \
    tests.cpp
HEADERS += \
    jingle.h \
    rpc.h \
    stun.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += codec.h sasl.h si.h
    SOURCES += codec.cpp sasl.cpp si.cpp
}
