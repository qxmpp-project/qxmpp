TEMPLATE = subdirs
SUBDIRS = \
    qxmpparchiveiq \
    qxmppbindiq \
    qxmppcallmanager \
    qxmppdataform \
    qxmppdiscoveryiq \
    qxmppentitytimeiq \
    qxmppiceconnection \
    qxmppiq \
    qxmppjingleiq \
    qxmppmessage \
    qxmppnonsaslauthiq \
    qxmpppresence \
    qxmpppubsubiq \
    qxmppregisteriq \
    qxmppresultset \
    qxmpprosteriq \
    qxmpprpciq \
    qxmpprtcppacket \
    qxmpprtppacket \
    qxmppserver \
    qxmppsessioniq \
    qxmppsocks \
    qxmppstanza \
    qxmppstreamfeatures \
    qxmppstunmessage \
    qxmpptransfermanager \
    qxmpputils \
    qxmppvcardiq \
    qxmppversioniq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppcodec
    SUBDIRS += qxmppsasl
    SUBDIRS += qxmppstreaminitiationiq
}
