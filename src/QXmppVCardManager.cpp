/*
 * Copyright (C) 2008-2010 The QXmpp developers
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


#include "QXmppVCardManager.h"
#include "QXmppStream.h"
#include "QXmppUtils.h"

QXmppVCardManager::QXmppVCardManager(QXmppStream* stream, QObject *parent)
    : QObject(parent),
    m_stream(stream),
    m_isClientVCardReceived(false)
{
    bool check = QObject::connect(m_stream, SIGNAL(vCardIqReceived(const QXmppVCard&)),
        this, SLOT(vCardIqReceived(const QXmppVCard&)));
    Q_ASSERT(check);
    Q_UNUSED(check);
}

/// This function requests the server for vCard of the specified jid.
/// Once received the signal vCardReceived() is emmitted.
///
/// \param jid Jid of the specific entry in the roster
///
void QXmppVCardManager::requestVCard(const QString& jid)
{
    QXmppVCard vcardIq(jid);
    m_stream->sendPacket(vcardIq);
}

void QXmppVCardManager::vCardIqReceived(const QXmppVCard& vcard)
{
    // self vCard received
    if(vcard.from().isEmpty())
    {
        m_clientVCard = vcard;
        m_isClientVCardReceived = true;
        emit clientVCardReceived();
    }

    emit vCardReceived(vcard);
}

/// Returns the vCard of the connected client.
///
/// \return QXmppVCard
///
const QXmppVCard& QXmppVCardManager::clientVCard() const
{
    return m_clientVCard;
}

/// Sets the vCard of the connected client.
///
/// \param clientVCard QXmppVCard
///
void QXmppVCardManager::setClientVCard(const QXmppVCard& clientVCard)
{
    m_clientVCard = clientVCard;
    m_clientVCard.setTo("");
    m_clientVCard.setFrom("");
    m_clientVCard.setType(QXmppIq::Set);
    m_stream->sendPacket(m_clientVCard);
}

/// This function requests the server for vCard of the connected user itself.
/// Once received the signal clientVCardReceived() is emmitted. Received vCard
/// can be get using clientVCard().
void QXmppVCardManager::requestClientVCard()
{
    requestVCard();
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

