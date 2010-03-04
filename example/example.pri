TEMPLATE = app

INCLUDEPATH += ../../source

QT += network xml

CONFIG += console debug_and_release

CONFIG(debug, debug|release) {
    LIBS += -L../../source -lQXmppClient_d
} else {
    LIBS += -L../../source -lQXmppClient
}
