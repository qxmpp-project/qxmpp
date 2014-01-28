include(../examples.pri)

TARGET = GuiClient
TEMPLATE = app

SOURCES += main.cpp \
    chatMsgGraphicsItem.cpp \
    chatGraphicsScene.cpp \
    chatGraphicsView.cpp \
    chatDialog.cpp \
    mainDialog.cpp \
    rosterItemModel.cpp \
    rosterItem.cpp \
    rosterItemSortFilterProxyModel.cpp \
    utils.cpp \
    rosterListView.cpp \
    searchLineEdit.cpp \
    statusWidget.cpp \
    signInStatusLabel.cpp \
    statusAvatarWidget.cpp \
    statusTextWidget.cpp \
    statusToolButton.cpp \
    vCardCache.cpp \
    profileDialog.cpp \
    capabilitiesCache.cpp \
    accountsCache.cpp \
    xmlConsoleDialog.cpp \
    aboutDialog.cpp

HEADERS += chatMsgGraphicsItem.h \
    chatGraphicsScene.h \
    chatGraphicsView.h \
    chatDialog.h \
    mainDialog.h \
    rosterItemModel.h \
    rosterItem.h \
    rosterItemSortFilterProxyModel.h \
    utils.h \
    rosterListView.h \
    searchLineEdit.h \
    statusWidget.h \
    signInStatusLabel.h \
    statusAvatarWidget.h \
    statusTextWidget.h \
    statusToolButton.h \
    vCardCache.h \
    profileDialog.h \
    capabilitiesCache.h \
    accountsCache.h \
    xmlConsoleDialog.h \
    aboutDialog.h

FORMS += mainDialog.ui \
    chatDialog.ui \
    statusWidget.ui \
    profileDialog.ui \
    xmlConsoleDialog.ui \
    aboutDialog.ui

QT += network \
    xml \
    widgets

RESOURCES += resources.qrc
