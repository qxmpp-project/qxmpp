# Build rules
docs.commands = cd doc && doxygen
docs.depends = doc/Doxyfile

# Install rules
htmldocs.files = doc/html
htmldocs.path = $$[QT_INSTALL_PREFIX]/share/doc/qxmpp
htmldocs.CONFIG += no_check_exist directory

QMAKE_EXTRA_TARGETS += docs
INSTALLS += htmldocs
