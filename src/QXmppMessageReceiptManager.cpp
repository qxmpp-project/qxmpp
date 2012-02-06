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

    QXmppMessage message;
    message.parse(stanza);

    // Handle receipts and cancel any further processing.
    if (!message.receiptId().isEmpty()) {
        emit messageDelivered(message.from(), message.receiptId());
        return true;
    }

    // If autoreceipt is enabled, we queue sending back receipt, otherwise
    // we just ignore the message. In either case, we don't cancel any
    // further processing.
    if (m_autoReceipt
        && message.isReceiptRequested()
        && !message.from().isEmpty()
        && !message.id().isEmpty()) {
        
        QMetaObject::invokeMethod(this,
                "sendReceipt",
                Q_ARG(QString, message.from()),
                Q_ARG(QString, message.id()));
    }

    return false;
}
