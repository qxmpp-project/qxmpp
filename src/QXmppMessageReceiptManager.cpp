/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Georg Rudoy
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

#include "QXmppMessageReceiptManager.h"

#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppClient.h"

/// Constructs a QXmppMessageReceiptManager to handle incoming and outgoing
/// message delivery receipts.

QXmppMessageReceiptManager::QXmppMessageReceiptManager()
    : QXmppClientExtension()
    , m_autoReceipt(true)
{
}

bool QXmppMessageReceiptManager::autoReceipt() const
{
    return m_autoReceipt;
}

void QXmppMessageReceiptManager::setAutoReceipt(bool autoReceipt)
{
    m_autoReceipt = autoReceipt;
}

/** Sends a receipt for the specified message.
 */
void QXmppMessageReceiptManager::sendReceipt(const QString &jid, const QString &id)
{
    QXmppMessage msg;
    msg.setTo(jid);
    msg.setReceiptId(id);
    client()->sendPacket(msg);
}

QStringList QXmppMessageReceiptManager::discoveryFeatures() const
{
    return QStringList(ns_message_receipts);
}

bool QXmppMessageReceiptManager::handleStanza(const QDomElement &stanza)
{
    if (stanza.tagName() != "message")
        return false;

    // Case 1: incoming receipt
    // This way we handle the receipt and cancel any further processing.
    const QDomElement &received = stanza.firstChildElement("received");
    if (received.namespaceURI() == ns_message_receipts)
    {
        QString id = received.attribute("id");

        // check if it's old-style XEP
        if (id.isEmpty())
            id = stanza.attribute("id");

        emit messageDelivered(stanza.attribute("from"), id);
        return true;
    }

    // Case 2: incoming message requesting receipt
    // If autoreceipt is enabled, we queue sending back receipt, otherwise
    // we just ignore the message. In either case, we don't cancel any
    // further processing.
    if (m_autoReceipt && stanza.firstChildElement("request").namespaceURI() == ns_message_receipts)
    {
        const QString &jid = stanza.attribute("from");
        const QString &id = stanza.attribute("id");

        // Send out receipt only if jid and id is not empty, otherwise fail
        // silently.
        if (!jid.isEmpty() && !id.isEmpty())
            QMetaObject::invokeMethod(this,
                    "sendReceipt",
                    Q_ARG(QString, jid),
                    Q_ARG(QString, id));
    }

    return false;
}
