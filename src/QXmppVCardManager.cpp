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


#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"
#include "QXmppVCardManager.h"

QXmppVCardManager::QXmppVCardManager()
    : QXmppClientExtension(),
    m_isClientVCardReceived(false)
{
}

QStringList QXmppVCardManager::discoveryFeatures() const
{
    // XEP-0054: vcard-temp
    return QStringList() << ns_vcard;
}

bool QXmppVCardManager::handleStanza(const QDomElement &element)
{
    if(element.tagName() == "iq" && QXmppVCardIq::isVCard(element))
    {
        QXmppVCardIq vCardIq;
        vCardIq.parse(element);

        if(vCardIq.from().isEmpty())
        {
            m_clientVCard = vCardIq;
            m_isClientVCardReceived = true;
            emit clientVCardReceived();
        }

        emit vCardReceived(vCardIq);

    // deprecated in 0.3.0 release
        QXmppVCard oldVCard(vCardIq);
        emit vCardReceived(oldVCard);

        return true;
    }

    return false;
}

/// This function requests the server for vCard of the specified jid.
/// Once received the signal vCardReceived() is emitted.
///
/// \param jid Jid of the specific entry in the roster
///
QString QXmppVCardManager::requestVCard(const QString& jid)
{
    QXmppVCardIq request(jid);
    if(client()->sendPacket(request))
        return request.id();
    else
        return QString();
}

/// Returns the vCard of the connected client.
///
/// \return QXmppVCard
///
const QXmppVCardIq& QXmppVCardManager::clientVCard() const
{
    return m_clientVCard;
}

/// Sets the vCard of the connected client.
///
/// \param clientVCard QXmppVCard
///
void QXmppVCardManager::setClientVCard(const QXmppVCardIq& clientVCard)
{
    m_clientVCard = clientVCard;
    m_clientVCard.setTo("");
    m_clientVCard.setFrom("");
    m_clientVCard.setType(QXmppIq::Set);
    client()->sendPacket(m_clientVCard);
}

/// This function requests the server for vCard of the connected user itself.
/// Once received the signal clientVCardReceived() is emitted. Received vCard
/// can be get using clientVCard().
QString QXmppVCardManager::requestClientVCard()
{
    return requestVCard();
}

/// Returns true if vCard of the connected client has been
/// received else false.
///
/// \return bool
///
bool QXmppVCardManager::isClientVCardReceived()
{
    return m_isClientVCardReceived;
}
