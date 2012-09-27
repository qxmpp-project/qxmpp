include(../tests.pri)

TARGET = tst_all

RESOURCES += tests.qrc
SOURCES += \
    jingle.cpp \
    rpc.cpp \
    tests.cpp
HEADERS += \
    jingle.h \
    rpc.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += sasl.h
    SOURCES += sasl.cpp
}
