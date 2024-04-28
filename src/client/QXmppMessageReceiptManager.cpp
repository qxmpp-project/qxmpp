// SPDX-FileCopyrightText: 2012 Georg Rudoy <0xd34df00d@gmail.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppMessageReceiptManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppE2eeMetadata.h"
#include "QXmppMessage.h"
#include "QXmppTask.h"
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
    return { ns_message_receipts.toString() };
}

bool QXmppMessageReceiptManager::handleMessage(const QXmppMessage &message)
{
    if (message.type() == QXmppMessage::Error) {
        return false;
    }
    // Handle receipts and cancel any further processing.
    if (!message.receiptId().isEmpty()) {
        // Buggy clients also mark carbon messages as received; to avoid this
        // we check whether sender and receiver have the same bare JID.
        if (QXmppUtils::jidToBareJid(message.from()) != QXmppUtils::jidToBareJid(message.to())) {
            Q_EMIT messageDelivered(message.from(), message.receiptId());
        }
        return true;
    }

    // If requested, send a receipt.
    if (message.isReceiptRequested() && !message.from().isEmpty() && !message.id().isEmpty()) {
        QXmppMessage receipt;
        receipt.setTo(message.from());
        receipt.setReceiptId(message.id());

        // Advise the server to store the receipt even if the message has no body.
        receipt.addHint(QXmppMessage::Store);

        client()->reply(std::move(receipt), message.e2eeMetadata());
    }

    // Continue processing.
    return false;
}
/// \endcond
