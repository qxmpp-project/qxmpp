TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmppdataform \
    qxmppiq \
    qxmppjingleiq \
    qxmppmessage \
    qxmpppresence \
    qxmppregisteriq \
    qxmppresultset \
    qxmpprosteriq \
    qxmpprtppacket \
    qxmppstanza \
    qxmppstunmessage \
    qxmppvcardiq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppcodec
    SUBDIRS += qxmppstreaminitiationiq
}
