TEMPLATE = app

TARGET = example_2_ibbTransfer

SOURCES +=  main.cpp \
            ibbClient.cpp

HEADERS +=  ibbClient.h

INCLUDEPATH += ../../source

QT += network xml

CONFIG += console debug_and_release

CONFIG(debug, debug|release) {
    LIBS += -L../../source/debug -lQXmppClient_d
 } else {
    LIBS += -L../../source/release -lQXmppClient
 }
