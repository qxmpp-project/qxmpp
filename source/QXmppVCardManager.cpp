/*
 * Copyright (C) 2008-2009 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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
#include "QXmppVCard.h"
#include "QXmppUtils.h"

QXmppVCardManager::QXmppVCardManager(QXmppClient* client) :
        m_client(client), QObject(client)
{
}

void QXmppVCardManager::requestVCard(const QString& jid)
{
    QXmppVCard vcardIq(jid);
    m_client->sendPacket(vcardIq);
}

void QXmppVCardManager::vCardIqReceived(const QXmppVCard& vcard)
{
    emit vCardReceived(vcard);
}

