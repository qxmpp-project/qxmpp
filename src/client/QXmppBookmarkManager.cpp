// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBookmarkManager.h"

#include "QXmppBookmarkSet.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppIq.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

// The QXmppPrivateStorageIq class represents an XML private storage IQ
// as defined by XEP-0049: Private XML Storage.
//
// FIXME: currently, we only handle bookmarks

class QXmppPrivateStorageIq : public QXmppIq
{
public:
    QXmppBookmarkSet bookmarks() const;
    void setBookmarks(const QXmppBookmarkSet &bookmark);

    static bool isPrivateStorageIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

private:
    QXmppBookmarkSet m_bookmarks;
};

QXmppBookmarkSet QXmppPrivateStorageIq::bookmarks() const
{
    return m_bookmarks;
}

void QXmppPrivateStorageIq::setBookmarks(const QXmppBookmarkSet &bookmarks)
{
    m_bookmarks = bookmarks;
}

bool QXmppPrivateStorageIq::isPrivateStorageIq(const QDomElement &element)
{
    return isIqType(element, u"query", ns_private) &&
        QXmppBookmarkSet::isBookmarkSet(element.firstChildElement().firstChildElement());
}

void QXmppPrivateStorageIq::parseElementFromChild(const QDomElement &element)
{
    const QDomElement queryElement = firstChildElement(element, u"query");
    m_bookmarks.parse(queryElement.firstChildElement());
}

void QXmppPrivateStorageIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("query"));
    writer->writeDefaultNamespace(toString65(ns_private));
    m_bookmarks.toXml(writer);
    writer->writeEndElement();
}

class QXmppBookmarkManagerPrivate
{
public:
    QXmppBookmarkSet bookmarks;
    QXmppBookmarkSet pendingBookmarks;
    QString pendingId;
    bool bookmarksReceived;
};

///
/// Constructs a new bookmark manager.
///
QXmppBookmarkManager::QXmppBookmarkManager()
    : d(new QXmppBookmarkManagerPrivate)
{
    d->bookmarksReceived = false;
}

QXmppBookmarkManager::~QXmppBookmarkManager() = default;

/// Returns true if the bookmarks have been received from the server,
/// false otherwise.
///
bool QXmppBookmarkManager::areBookmarksReceived() const
{
    return d->bookmarksReceived;
}

/// Returns the bookmarks stored on the server.
///
/// Before calling this method, check that the bookmarks
/// have indeed been received by calling areBookmarksReceived().
///

QXmppBookmarkSet QXmppBookmarkManager::bookmarks() const
{
    return d->bookmarks;
}

/// Stores the bookmarks on the server.
///
/// \param bookmarks

bool QXmppBookmarkManager::setBookmarks(const QXmppBookmarkSet &bookmarks)
{
    QXmppPrivateStorageIq iq;
    iq.setType(QXmppIq::Set);
    iq.setBookmarks(bookmarks);
    if (!client()->sendPacket(iq)) {
        return false;
    }

    d->pendingBookmarks = bookmarks;
    d->pendingId = iq.id();
    return true;
}

/// \cond
bool QXmppBookmarkManager::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() == u"iq") {
        if (QXmppPrivateStorageIq::isPrivateStorageIq(stanza)) {
            QXmppPrivateStorageIq iq;
            iq.parse(stanza);

            if (iq.type() == QXmppIq::Result) {
                d->bookmarks = iq.bookmarks();
                d->bookmarksReceived = true;
                Q_EMIT bookmarksReceived(d->bookmarks);
            }
            return true;
        } else if (!d->pendingId.isEmpty() && stanza.attribute(u"id"_s) == d->pendingId) {
            QXmppIq iq;
            iq.parse(stanza);
            if (iq.type() == QXmppIq::Result) {
                d->bookmarks = d->pendingBookmarks;
                Q_EMIT bookmarksReceived(d->bookmarks);
            }
            d->pendingId = QString();
            return true;
        }
    }
    return false;
}

void QXmppBookmarkManager::onRegistered(QXmppClient *client)
{
    connect(client, &QXmppClient::connected,
            this, &QXmppBookmarkManager::slotConnected);

    connect(client, &QXmppClient::disconnected,
            this, &QXmppBookmarkManager::slotDisconnected);
}

void QXmppBookmarkManager::onUnregistered(QXmppClient *client)
{
    disconnect(client, &QXmppClient::connected,
               this, &QXmppBookmarkManager::slotConnected);

    disconnect(client, &QXmppClient::disconnected,
               this, &QXmppBookmarkManager::slotDisconnected);
}
/// \endcond

void QXmppBookmarkManager::slotConnected()
{
    QXmppPrivateStorageIq iq;
    iq.setType(QXmppIq::Get);
    client()->sendPacket(iq);
}

void QXmppBookmarkManager::slotDisconnected()
{
    d->bookmarks = QXmppBookmarkSet();
    d->bookmarksReceived = false;
}
