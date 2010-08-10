include(qxmpp.pri)

TEMPLATE = subdirs

SUBDIRS = source \
          tests \
          examples

CONFIG += ordered

include(doc/doc.pri)

# Source distribution
QXMPP_ARCHIVE = qxmpp-$$QXMPP_VERSION
dist.commands = \
    rm -rf $$QXMPP_ARCHIVE && \
    svn export . $$QXMPP_ARCHIVE && \
    cp -rf doc/html $$QXMPP_ARCHIVE && \
    tar czf $${QXMPP_ARCHIVE}.tar.gz $$QXMPP_ARCHIVE && \
    rm -rf $$QXMPP_ARCHIVE
dist.depends = docs
QMAKE_EXTRA_TARGETS += dist

