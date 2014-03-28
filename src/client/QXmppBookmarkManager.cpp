/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include <QDomElement>

#include "QXmppBookmarkManager.h"
#include "QXmppBookmarkSet.h"
#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppIq.h"
#include "QXmppUtils.h"

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
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

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
    const QDomElement queryElement = element.firstChildElement("query");
    return queryElement.namespaceURI() == ns_private &&
           QXmppBookmarkSet::isBookmarkSet(queryElement.firstChildElement());
}

void QXmppPrivateStorageIq::parseElementFromChild(const QDomElement &element)
{
    const QDomElement queryElement = element.firstChildElement("query");
    m_bookmarks.parse(queryElement.firstChildElement());
}

void QXmppPrivateStorageIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_private);
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

/// Constructs a new bookmark manager.
///
QXmppBookmarkManager::QXmppBookmarkManager()
    : d(new QXmppBookmarkManagerPrivate)
{
    d->bookmarksReceived = false;
}

/// Destroys a bookmark manager.
///
QXmppBookmarkManager::~QXmppBookmarkManager()
{
    delete d;
}

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
    if (!client()->sendPacket(iq))
        return false;

    d->pendingBookmarks = bookmarks;
    d->pendingId = iq.id();
    return true;
}

/// \cond
void QXmppBookmarkManager::setClient(QXmppClient *client)
{
    bool check;
    Q_UNUSED(check);

    QXmppClientExtension::setClient(client);

    check = connect(client, SIGNAL(connected()),
                    this, SLOT(slotConnected()));
    Q_ASSERT(check);

    check = connect(client, SIGNAL(disconnected()),
                    this, SLOT(slotDisconnected()));
    Q_ASSERT(check);
}

bool QXmppBookmarkManager::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() == "iq")
    {
        if (QXmppPrivateStorageIq::isPrivateStorageIq(stanza))
        {
            QXmppPrivateStorageIq iq;
            iq.parse(stanza);

            if (iq.type() == QXmppIq::Result)
            {
                d->bookmarks = iq.bookmarks();
                d->bookmarksReceived = true;
                emit bookmarksReceived(d->bookmarks);
            }
            return true;
        }
        else if (!d->pendingId.isEmpty() && stanza.attribute("id") == d->pendingId)
        {
            QXmppIq iq;
            iq.parse(stanza);
            if (iq.type() == QXmppIq::Result)
            {
                d->bookmarks = d->pendingBookmarks;
                emit bookmarksReceived(d->bookmarks);
            }
            d->pendingId = QString();
            return true;
        }
    }
    return false;
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

