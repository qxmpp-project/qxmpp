include(../tests.pri)

TARGET = tst_all

RESOURCES += tests.qrc
SOURCES += tests.cpp
HEADERS += tests.h

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    HEADERS += sasl.h
    SOURCES += sasl.cpp
}
