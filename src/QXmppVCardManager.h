/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#ifndef QXMPPVCARDMANAGER_H
#define QXMPPVCARDMANAGER_H

#include <QObject>

#include "QXmppClientExtension.h"
#include "QXmppVCardIq.h"

#define QXMPP_SUPRESS_INTERNAL_VCARD_WARNING
#include "QXmppVCard.h"
#undef QXMPP_SUPRESS_INTERNAL_VCARD_WARNING

/// \brief The QXmppVCardManager class gets/sets XMPP vCards. It is an
/// implentation of <B>XEP-0054: vcard-temp</B>.
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

class QXmppVCardManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppVCardManager();
    QString requestVCard(const QString& bareJid = "");

    const QXmppVCardIq& clientVCard() const;
    void setClientVCard(const QXmppVCardIq&);
    QString requestClientVCard();
    bool isClientVCardReceived();

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

    /// \cond
// deprecated in release 0.3.0
    void vCardReceived(const QXmppVCard&);
    /// \endcond

private:
    QXmppVCardIq m_clientVCard;  ///< Stores the vCard of the connected client
    bool m_isClientVCardReceived;
};

#endif // QXMPPVCARDMANAGER_H
