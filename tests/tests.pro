TEMPLATE = subdirs
SUBDIRS = \
    all \
    qxmppiq \
    qxmppmessage \
    qxmpppresence \
    qxmppvcardiq

!isEmpty(QXMPP_AUTOTEST_INTERNAL) {
    SUBDIRS += qxmppstreaminitiationiq
}
