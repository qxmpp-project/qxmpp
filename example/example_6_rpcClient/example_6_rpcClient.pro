TEMPLATE = app
TARGET = example_6_rpcClient
SOURCES += main.cpp \
    rpcClient.cpp 
HEADERS += rpcClient.h 
INCLUDEPATH += ../../source
QT += network \
    xml
CONFIG += console \
    debug_and_release
CONFIG(debug, debug|release):LIBS += -L../../source \
    -lQXmppClient_d
else:LIBS += -L../../source \
    -lQXmppClient
OTHER_FILES += README
