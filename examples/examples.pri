include(../qxmpp.pri)

TEMPLATE = app
CONFIG += console

INCLUDEPATH += $$QXMPP_INCLUDE_DIR
LIBS += $$QXMPP_LIBS
PRE_TARGETDEPS += $$QXMPP_LIBRARY_FILE

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
