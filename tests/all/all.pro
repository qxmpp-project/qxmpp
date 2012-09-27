include(../tests.pri)

TARGET = tst_all

SOURCES += tests.cpp
HEADERS += tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += sasl.h
    SOURCES += sasl.cpp
}
