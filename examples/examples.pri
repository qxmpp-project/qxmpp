include(../qxmpp.pri)

TEMPLATE = app

QT += network xml

CONFIG += console

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += -L$$QXMPP_LIBRARY_DIR -l$$QXMPP_LIB
PRE_TARGETDEPS += $${QXMPP_LIBRARY_DIR}/lib$${QXMPP_LIB}.a

