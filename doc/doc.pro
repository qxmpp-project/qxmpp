include(../qxmpp.pri)

TEMPLATE = app
CONFIG += console
QT -= gui

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
TARGET = doxyfilter
SOURCES += doxyfilter.cpp

windows {
    DOXYFILTER = $${TARGET}.exe
} else {
    DOXYFILTER = ./$${TARGET}
}

# Build rules
docs.commands = $${DOXYFILTER} -g && $${DOXYFILTER}
docs.depends = $${TARGET}

QMAKE_CLEAN += Doxyfile
QMAKE_EXTRA_TARGETS += docs
