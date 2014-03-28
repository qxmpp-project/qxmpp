/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
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

#ifndef QXMPPMESSAGERECEIPTMANAGER_H
#define QXMPPMESSAGERECEIPTMANAGER_H

#include "QXmppClientExtension.h"

/// \brief The QXmppMessageReceiptManager class makes it possible to
/// send and receive message delivery receipts as defined in
/// XEP-0184: Message Delivery Receipts.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppMessageReceiptManager : public QXmppClientExtension
{
    Q_OBJECT
public:
    QXmppMessageReceiptManager();

    /// \cond
    virtual QStringList discoveryFeatures() const;
    virtual bool handleStanza(const QDomElement &stanza);
    /// \endcond

signals:
    /// This signal is emitted when receipt for the message with the
    /// given id is received. The id could be previously obtained by
    /// calling QXmppMessage::id().
    void messageDelivered(const QString &jid, const QString &id);
};

#endif // QXMPPMESSAGERECEIPTMANAGER_H
