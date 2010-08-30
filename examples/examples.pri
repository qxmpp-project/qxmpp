include(../qxmpp.pri)

TEMPLATE = app

QT += network xml

CONFIG += console

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += -L$$QXMPP_LIBRARY_DIR -l$$QXMPP_LIBRARY_NAME

# DNS support
unix {
    LIBS += -lresolv
}
win32 {
    LIBS += -ldnsapi
}


# FIXME: we need to express a dependency on the library, but the file name
# depends on the platform and whether the library is static or dynamic
# PRE_TARGETDEPS += $${QXMPP_LIBRARY_DIR}/lib$${QXMPP_LIB}.a

