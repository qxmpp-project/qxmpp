include(qxmpp.pri)

CONFIG += ordered
TEMPLATE = subdirs

SUBDIRS = src
android | ios {
} else {
    SUBDIRS += tests examples
}

!isEmpty(QXMPP_USE_DOXYGEN) {
    docs.commands = cd doc/ && $(MAKE) docs
    docs.depends = sub-doc
    docs.files = doc/html
    docs.path = $$PREFIX/share/doc/qxmpp
    docs.CONFIG += no_check_exist directory

    INSTALLS += docs
    QMAKE_EXTRA_TARGETS += docs
    SUBDIRS += doc
}
