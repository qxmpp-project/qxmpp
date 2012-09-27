TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmppdataform \
    qxmppiq \
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
    SUBDIRS += qxmppstreaminitiationiq
}
