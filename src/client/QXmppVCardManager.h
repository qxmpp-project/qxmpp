/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#ifndef QXMPPVCARDMANAGER_H
#define QXMPPVCARDMANAGER_H

#include "QXmppClientExtension.h"

class QXmppVCardIq;
class QXmppVCardManagerPrivate;

/// \brief The QXmppVCardManager class gets/sets XMPP vCards. It is an
/// implementation of XEP-0054: vcard-temp.
///
/// \note It's object should not be created using it's constructor. Instead
/// QXmppClient::vCardManager() should be used to get the reference of instantiated
/// object this class.
///
/// <B>Getting vCards of entries in Roster:</B><BR>
/// It doesn't store vCards of the JIDs in the roster of connected user. Instead
/// client has to request for a particular vCard using requestVCard(). And connect to
/// the signal vCardReceived() to get the requested vCard.
///
/// <B>Getting vCard of the connected client:</B><BR>
/// For getting the vCard of the connected user itself. Client can call requestClientVCard()
/// and on the signal clientVCardReceived() it can get its vCard using clientVCard().
///
/// <B>Setting vCard of the client:</B><BR>
/// Using setClientVCard() client can set its vCard.
///
/// \note Client can't set/change vCards of roster entries.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppVCardManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppVCardManager();
    ~QXmppVCardManager();

    QString requestVCard(const QString& bareJid = QString());

    const QXmppVCardIq& clientVCard() const;
    void setClientVCard(const QXmppVCardIq&);

    QString requestClientVCard();
    bool isClientVCardReceived() const;

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when the requested vCard is received
    /// after calling the requestVCard() function.
    void vCardReceived(const QXmppVCardIq&);

    /// This signal is emitted when the client's vCard is received
    /// after calling the requestClientVCard() function.
    void clientVCardReceived();

private:
    QXmppVCardManagerPrivate *d;
};

#endif // QXMPPVCARDMANAGER_H
