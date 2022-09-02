// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPBOOKMARKMANAGER_H
#define QXMPPBOOKMARKMANAGER_H

#include "QXmppClientExtension.h"

#include <QUrl>

class QXmppBookmarkManagerPrivate;
class QXmppBookmarkSet;

/// \brief The QXmppBookmarkManager class allows you to store and retrieve
/// bookmarks as defined by \xep{0048}: Bookmarks.
///

class QXMPP_EXPORT QXmppBookmarkManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppBookmarkManager();
    ~QXmppBookmarkManager() override;

    bool areBookmarksReceived() const;
    QXmppBookmarkSet bookmarks() const;
    bool setBookmarks(const QXmppBookmarkSet &bookmarks);

    /// \cond
    bool handleStanza(const QDomElement &stanza) override;
    /// \endcond

Q_SIGNALS:
    /// This signal is emitted when bookmarks are received.
    void bookmarksReceived(const QXmppBookmarkSet &bookmarks);

protected:
    /// \cond
    void setClient(QXmppClient *client) override;
    /// \endcond

private Q_SLOTS:
    void slotConnected();
    void slotDisconnected();

private:
    const std::unique_ptr<QXmppBookmarkManagerPrivate> d;
};

#endif
