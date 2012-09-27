TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmppdataform \
    qxmppiq \
    qxmppmessage \
    qxmpppresence \
    qxmppvcardiq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppstreaminitiationiq
}
