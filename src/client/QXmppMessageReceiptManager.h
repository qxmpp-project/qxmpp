// SPDX-FileCopyrightText: 2019 Georg Rudoy <0xd34df00d@gmail.com>
// SPDX-FileCopyrightText: 2012 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMESSAGERECEIPTMANAGER_H
#define QXMPPMESSAGERECEIPTMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppMessageHandler.h"

///
/// \brief The QXmppMessageReceiptManager class makes it possible to
/// send and receive message delivery receipts as defined in
/// \xep{0184}: Message Delivery Receipts.
///
/// \ingroup Managers
///
class QXMPP_EXPORT QXmppMessageReceiptManager : public QXmppClientExtension, public QXmppMessageHandler
{
    Q_OBJECT
public:
    QXmppMessageReceiptManager();

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleMessage(const QXmppMessage &) override;
    /// \endcond

Q_SIGNALS:
    /// This signal is emitted when receipt for the message with the
    /// given id is received. The id could be previously obtained by
    /// calling QXmppMessage::id().
    void messageDelivered(const QString &jid, const QString &id);
};

#endif  // QXMPPMESSAGERECEIPTMANAGER_H
