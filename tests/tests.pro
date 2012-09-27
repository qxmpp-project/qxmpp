TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmpparchiveiq \
    qxmppdataform \
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
    qxmppvcardiq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppcodec
    SUBDIRS += qxmppsasl
    SUBDIRS += qxmppstreaminitiationiq
}
