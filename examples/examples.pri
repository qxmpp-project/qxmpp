include(../qxmpp.pri)

TEMPLATE = app
CONFIG += console

QMAKE_LIBDIR += ../../src
QMAKE_RPATHDIR += $$OUT_PWD/../../src
INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += $$QXMPP_LIBS

# Symbian packaging rules
symbian {
    vendorinfo = \
        "; Localised Vendor name" \
        "%{\"QXmpp\"}" \
        " " \
        "; Unique Vendor name" \
        ":\"QXmpp\"" \
        " "

    examples_deployment.pkg_prerules += vendorinfo
    DEPLOYMENT += examples_deployment

    TARGET.CAPABILITY = "NetworkServices"
}
