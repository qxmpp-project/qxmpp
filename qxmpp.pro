include(qxmpp.pri)

TEMPLATE = subdirs

SUBDIRS = src

android | ios {
} else {
    SUBDIRS += tests examples doc
    INSTALLS += docs
}

CONFIG += ordered

# HTML documentation
docs.commands = cd doc/ && $(MAKE) docs
docs.depends = sub-doc
docs.files = doc/html
docs.path = $$PREFIX/share/doc/qxmpp
docs.CONFIG += no_check_exist directory

QMAKE_EXTRA_TARGETS += docs
