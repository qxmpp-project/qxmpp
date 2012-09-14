include(../examples.pri)

CONFIG += mobility
MOBILITY += multimedia

TARGET = example_4_callHandling

SOURCES += example_4_callHandling.cpp

HEADERS += example_4_callHandling.h

# Symbian packaging rules
symbian {
    TARGET.CAPABILITY = "UserEnvironment"
}
