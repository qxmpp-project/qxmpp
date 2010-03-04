include(../example.pri)

TARGET = example_5_rpcInterface

SOURCES += main.cpp \
           rpcClient.cpp \
           remoteinterface.cpp

HEADERS += rpcClient.h \
           remoteinterface.h

OTHER_FILES += README
