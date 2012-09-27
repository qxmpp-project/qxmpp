include(../tests.pri)

TARGET = tst_all

RESOURCES += tests.qrc
SOURCES += \
    rpc.cpp \
    tests.cpp
HEADERS += \
    rpc.h \
    tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += sasl.h
    SOURCES += sasl.cpp
}
