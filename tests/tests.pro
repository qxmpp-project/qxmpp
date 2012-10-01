TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmpparchiveiq \
    qxmppbindiq \
    qxmppdataform \
    qxmppdiscoveryiq \
    qxmppiq \
    qxmppjingleiq \
    qxmppmessage \
    qxmpppresence \
    qxmpppubsubiq \
    qxmppregisteriq \
    qxmppresultset \
    qxmpprosteriq \
    qxmpprpciq \
    qxmpprtppacket \
    qxmppserver \
    qxmppstanza \
    qxmppstunmessage \
    qxmpputils \
    qxmppvcardiq \
    qxmppversioniq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppcodec
    SUBDIRS += qxmppsasl
    SUBDIRS += qxmppstreaminitiationiq
}
