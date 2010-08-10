include(qxmpp.pri)

TEMPLATE = subdirs

SUBDIRS = source \
          tests \
          examples

CONFIG += ordered

include(doc/doc.pri)
