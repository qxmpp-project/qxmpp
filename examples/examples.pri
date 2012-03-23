include(../qxmpp.pri)

TEMPLATE = app
CONFIG += console

INCLUDEPATH += $$QXMPP_INCLUDEPATH
LIBS += -L../../src $$QXMPP_LIBS

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
