include(../qxmpp.pri)

TEMPLATE = app
CONFIG += console
QT -= gui

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
TARGET = doxyfilter
SOURCES += doxyfilter.cpp
