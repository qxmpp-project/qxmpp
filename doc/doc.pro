include(../qxmpp.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT -= gui

INCLUDEPATH += $$QXMPP_INCLUDEPATH
TARGET = doxyfilter
SOURCES += doxyfilter.cpp

windows {
    DOXYFILTER = $${TARGET}.exe
} else {
    DOXYFILTER = ./$${TARGET}
}

# Build rules
docs.commands = $${DOXYFILTER} -g $${PWD} && $${DOXYFILTER}
docs.depends = $${TARGET}

QMAKE_CLEAN += Doxyfile doxygen_sqlite3.db
unix:QMAKE_DISTCLEAN += -r html
QMAKE_EXTRA_TARGETS += docs
