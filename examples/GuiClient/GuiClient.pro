include(../examples.pri)

TARGET = GuiClient
TEMPLATE = app

SOURCES += main.cpp \
    messageGraphicsItem.cpp \
    chatGraphicsScene.cpp \
    chatGraphicsView.cpp \
    chatDialog.cpp \
    mainDialog.cpp \
    rosterItemModel.cpp \
    rosterItem.cpp \
    rosterItemSortFilterProxyModel.cpp \
    utils.cpp \
    customListView.cpp \
    searchLineEdit.cpp \
    statusWidget.cpp \
    customPushButton.cpp \
    customLabel.cpp \
    avatarWidget.cpp \
    statusTextWidget.cpp \
    customToolButton.cpp \
    vCardManager.cpp
HEADERS += messageGraphicsItem.h \
    chatGraphicsScene.h \
    chatGraphicsView.h \
    chatDialog.h \
    mainDialog.h \
    rosterItemModel.h \
    rosterItem.h \
    rosterItemSortFilterProxyModel.h \
    utils.h \
    customListView.h \
    searchLineEdit.h \
    statusWidget.h \
    customPushButton.h \
    customLabel.h \
    avatarWidget.h \
    statusTextWidget.h \
    customToolButton.h \
    vCardManager.h

FORMS += mainDialog.ui \
    chatDialog.ui \
    statusWidget.ui

QT += network \
    xml

RESOURCES += resources.qrc
