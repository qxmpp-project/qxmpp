/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
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

#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppMucIq.h"
#include "QXmppMucManager.h"
#include "QXmppStream.h"
#include "QXmppUtils.h"

QXmppMucManager::QXmppMucManager(QXmppStream* stream, QObject *parent)
    : QObject(parent),
    m_stream(stream)
{
    bool check = connect(stream, SIGNAL(mucAdminIqReceived(const QXmppMucAdminIq&)),
        this, SLOT(mucAdminIqReceived(const QXmppMucAdminIq&)));

    check = connect(stream, SIGNAL(mucOwnerIqReceived(const QXmppMucOwnerIq&)),
        this, SLOT(mucOwnerIqReceived(const QXmppMucOwnerIq&)));
    Q_ASSERT(check);

    check = QObject::connect(m_stream, SIGNAL(presenceReceived(const QXmppPresence&)),
        this, SLOT(presenceReceived(const QXmppPresence&)));
}

/// Joins the given chat room with the requested nickname.
///
/// \return true if the request was sent, false otherwise
///

bool QXmppMucManager::joinRoom(const QString &bareJid, const QString &nickName)
{
    QXmppPresence packet;
    packet.setTo(bareJid + "/" + nickName);
    packet.setType(QXmppPresence::Available);
    QXmppElement x;
    x.setTagName("x");
    x.setAttribute("xmlns", ns_muc);
    packet.setExtensions(x);
    if (m_stream->sendPacket(packet))
    {
        m_nickNames[bareJid] = nickName;
        return true;
    } else {
        return false;
    }
}

/// Leaves the given chat room.
///
/// \return true if the request was sent, false otherwise
///

bool QXmppMucManager::leaveRoom(const QString &bareJid)
{
    if (!m_nickNames.contains(bareJid))
        return false;
    QString nickName = m_nickNames.take(bareJid);
    QXmppPresence packet;
    packet.setTo(bareJid + "/" + nickName);
    packet.setType(QXmppPresence::Unavailable);
    return m_stream->sendPacket(packet);
}

/// Retrieves the list of participants for the given room.
///

QMap<QString, QXmppPresence> QXmppMucManager::roomParticipants(const QString& bareJid) const
{
    return m_participants.value(bareJid);
}

/// Request the configuration form for the given room.
///
/// \return true if the request was sent, false otherwise
///
/// \sa roomConfigurationReceived()
///

bool QXmppMucManager::requestRoomConfiguration(const QString &bareJid)
{
    QXmppMucOwnerIq iq;
    iq.setTo(bareJid);
    return m_stream->sendPacket(iq);
}

/// Send the configuration form for the given room.
///
/// \return true if the request was sent, false otherwise
///

bool QXmppMucManager::setRoomConfiguration(const QString &bareJid, const QXmppDataForm &form)
{
    QXmppMucOwnerIq iqPacket;
    iqPacket.setType(QXmppIq::Set);
    iqPacket.setTo(bareJid);
    iqPacket.setForm(form);
    return m_stream->sendPacket(iqPacket);
}

/// Send a message to a chat room.
///
/// \return true if the message was sent, false otherwise
///

bool QXmppMucManager::sendMessage(const QString &bareJid, const QString &text)
{
    if (!m_nickNames.contains(bareJid))
    {
        qWarning("Cannot send message to unknown chat room");
        return false;
    }
    QXmppMessage msg;
    msg.setBody(text);
    msg.setFrom(bareJid + "/" + m_nickNames[bareJid]);
    msg.setTo(bareJid);
    msg.setType(QXmppMessage::GroupChat);
    return m_stream->sendPacket(msg);
}

void QXmppMucManager::mucAdminIqReceived(const QXmppMucAdminIq &iq)
{
}

void QXmppMucManager::mucOwnerIqReceived(const QXmppMucOwnerIq &iq)
{
    if (iq.type() == QXmppIq::Result && !iq.form().isNull())
        emit roomConfigurationReceived(iq.from(), iq.form());
}

void QXmppMucManager::presenceReceived(const QXmppPresence &presence)
{
    QString jid = presence.from();
    QString bareJid = jidToBareJid(jid);
    QString resource = jidToResource(jid);
    if (!m_nickNames.contains(bareJid))
        return;

    if (presence.type() == QXmppPresence::Available)
        m_participants[bareJid][resource] = presence;
    else if (presence.type() == QXmppPresence::Unavailable)
        m_participants[bareJid].remove(resource);
    else
        return;

    emit roomParticipantChanged(bareJid, resource);
}

