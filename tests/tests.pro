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
    qxmpprpciq \
    qxmpprtppacket \
    qxmppstanza \
    qxmppstunmessage \
    qxmpputils \
    qxmppvcardiq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppcodec
    SUBDIRS += qxmppstreaminitiationiq
}
