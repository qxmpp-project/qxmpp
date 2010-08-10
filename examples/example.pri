include(../qxmpp.pri)

TEMPLATE = app

INCLUDEPATH += $$QXMPP_INCLUDE_DIR

QT += network xml

CONFIG += console

LIBS += -L$$QXMPP_LIBRARY_DIR -l$$QXMPP_LIB
PRE_TARGETDEPS += $${QXMPP_LIBRARY_DIR}/lib$${QXMPP_LIB}.a

