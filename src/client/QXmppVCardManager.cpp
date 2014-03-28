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


#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppUtils.h"
#include "QXmppVCardIq.h"
#include "QXmppVCardManager.h"

class QXmppVCardManagerPrivate
{
public:
    QXmppVCardIq clientVCard;
    bool isClientVCardReceived;
};

QXmppVCardManager::QXmppVCardManager()
    : d(new QXmppVCardManagerPrivate)
{
    d->isClientVCardReceived = false;
}

QXmppVCardManager::~QXmppVCardManager()
{
    delete d;
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
    return d->clientVCard;
}

/// Sets the vCard of the connected client.
///
/// \param clientVCard QXmppVCard
///
void QXmppVCardManager::setClientVCard(const QXmppVCardIq& clientVCard)
{
    d->clientVCard = clientVCard;
    d->clientVCard.setTo("");
    d->clientVCard.setFrom("");
    d->clientVCard.setType(QXmppIq::Set);
    client()->sendPacket(d->clientVCard);
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
bool QXmppVCardManager::isClientVCardReceived() const
{
    return d->isClientVCardReceived;
}

/// \cond
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

        if (vCardIq.from().isEmpty()) {
            d->clientVCard = vCardIq;
            d->isClientVCardReceived = true;
            emit clientVCardReceived();
        }

        emit vCardReceived(vCardIq);

        return true;
    }

    return false;
}
/// \endcond
