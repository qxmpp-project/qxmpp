include(qxmpp.pri)

TEMPLATE = subdirs

SUBDIRS = src

android {
} else {
    SUBDIRS += tests examples doc
    INSTALLS += htmldocs
}

CONFIG += ordered

# Documentation generation
docs.commands = cd doc/ && $(QMAKE) && $(MAKE) docs

# Install rules
htmldocs.files = doc/html
htmldocs.path = $$PREFIX/share/doc/qxmpp
htmldocs.CONFIG += no_check_exist directory

QMAKE_EXTRA_TARGETS += docs
