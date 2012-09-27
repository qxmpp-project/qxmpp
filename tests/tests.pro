TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmppdataform \
    qxmppiq \
    qxmppmessage \
    qxmpppresence \
    qxmppresultset \
    qxmpprtppacket \
    qxmppstanza \
    qxmppvcardiq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppstreaminitiationiq
}
