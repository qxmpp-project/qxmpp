/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Georg Rudoy
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

#include "QXmppMessageReceiptManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include <QDomElement>

/// Constructs a QXmppMessageReceiptManager to handle incoming and outgoing
/// message delivery receipts.

QXmppMessageReceiptManager::QXmppMessageReceiptManager()
    : QXmppClientExtension()
{
}

/// \cond
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

    if (message.type() == QXmppMessage::Error)
        return false;

    // Handle receipts and cancel any further processing.
    if (!message.receiptId().isEmpty()) {
        // Buggy clients also mark carbon messages as received; to avoid this
        // we check whether sender and receiver have the same bare JID.
        if (QXmppUtils::jidToBareJid(message.from()) != QXmppUtils::jidToBareJid(message.to())) {
            emit messageDelivered(message.from(), message.receiptId());
        }
        return true;
    }

    // If requested, send a receipt.
    if (message.isReceiptRequested() && !message.from().isEmpty() && !message.id().isEmpty()) {
        QXmppMessage receipt;
        receipt.setTo(message.from());
        receipt.setReceiptId(message.id());
        client()->sendPacket(receipt);
    }

    // Continue processing.
    return false;
}
/// \endcond
