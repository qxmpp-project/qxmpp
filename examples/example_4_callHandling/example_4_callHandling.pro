include(../examples.pri)

CONFIG += mobility
MOBILITY += multimedia

TARGET = example_4_callHandling

SOURCES +=  main.cpp \
            xmppClient.cpp

HEADERS +=  xmppClient.h

# Symbian packaging rules
symbian {
    TARGET.CAPABILITY = "UserEnvironment"
}
