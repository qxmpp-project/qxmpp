/*
 * Copyright (C) 2008-2011 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#include <QDomElement>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppMucIq.h"
#include "QXmppMucManager.h"
#include "QXmppUtils.h"

void QXmppMucManager::setClient(QXmppClient* client)
{
    QXmppClientExtension::setClient(client);

    bool check = connect(client, SIGNAL(messageReceived(QXmppMessage)),
        this, SLOT(messageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = QObject::connect(client, SIGNAL(presenceReceived(QXmppPresence)),
        this, SLOT(presenceReceived(QXmppPresence)));
    Q_ASSERT(check);
}

QStringList QXmppMucManager::discoveryFeatures() const
{
    // XEP-0045: Multi-User Chat
    return QStringList()
        << ns_muc
        << ns_muc_admin
        << ns_muc_owner
        << ns_muc_user;
}

bool QXmppMucManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq")
    {
        if (QXmppMucAdminIq::isMucAdminIq(element))
        {
            QXmppMucAdminIq iq;
            iq.parse(element);
            mucAdminIqReceived(iq);
            return true;
        }
        else if (QXmppMucOwnerIq::isMucOwnerIq(element))
        {
            QXmppMucOwnerIq iq;
            iq.parse(element);
            mucOwnerIqReceived(iq);
            return true;
        }
    }
    return false;
}

/// Joins the given chat room with the requested nickname.
///
/// \param roomJid
/// \param nickName
/// \param password an optional password if the room is password-protected
///
/// \return true if the request was sent, false otherwise
///

bool QXmppMucManager::joinRoom(const QString &roomJid, const QString &nickName, const QString &password)
{
    QXmppPresence packet;
    packet.setTo(roomJid + "/" + nickName);
    packet.setType(QXmppPresence::Available);
    QXmppElement x;
    x.setTagName("x");
    x.setAttribute("xmlns", ns_muc);
    if (!password.isEmpty())
    {
        QXmppElement p;
        p.setTagName("password");
        p.setValue(password);
        x.appendChild(p);
    }
    packet.setExtensions(x);
    if (client()->sendPacket(packet))
    {
        m_nickNames[roomJid] = nickName;
        return true;
    } else {
        return false;
    }
}

/// Leaves the given chat room.
///
/// \param roomJid
///
/// \return true if the request was sent, false otherwise
///

bool QXmppMucManager::leaveRoom(const QString &roomJid)
{
    if (!m_nickNames.contains(roomJid))
        return false;
    QString nickName = m_nickNames.take(roomJid);
    QXmppPresence packet;
    packet.setTo(roomJid + "/" + nickName);
    packet.setType(QXmppPresence::Unavailable);
    return client()->sendPacket(packet);
}

/// Retrieves the list of participants for the given room.
///
/// \param roomJid
///

QMap<QString, QXmppPresence> QXmppMucManager::roomParticipants(const QString& roomJid) const
{
    return m_participants.value(roomJid);
}

/// Request the configuration form for the given room.
///
/// \param roomJid
///
/// \return true if the request was sent, false otherwise
///
/// \sa roomConfigurationReceived()

bool QXmppMucManager::requestRoomConfiguration(const QString &roomJid)
{
    QXmppMucOwnerIq iq;
    iq.setTo(roomJid);
    return client()->sendPacket(iq);
}

/// Send the configuration form for the given room.
///
/// \param roomJid
/// \param form
///
/// \return true if the request was sent, false otherwise

bool QXmppMucManager::setRoomConfiguration(const QString &roomJid, const QXmppDataForm &form)
{
    QXmppMucOwnerIq iqPacket;
    iqPacket.setType(QXmppIq::Set);
    iqPacket.setTo(roomJid);
    iqPacket.setForm(form);
    return client()->sendPacket(iqPacket);
}

/// Request the room's permissions.
///
/// \param roomJid
///
/// \return true if the request was sent, false otherwise

bool QXmppMucManager::requestRoomPermissions(const QString &roomJid)
{
    QList<QXmppMucAdminIq::Item::Affiliation> affiliations;
    affiliations << QXmppMucAdminIq::Item::OwnerAffiliation;
    affiliations << QXmppMucAdminIq::Item::AdminAffiliation;
    affiliations << QXmppMucAdminIq::Item::MemberAffiliation;
    affiliations << QXmppMucAdminIq::Item::OutcastAffiliation;
    foreach (QXmppMucAdminIq::Item::Affiliation affiliation, affiliations)
    {
        QXmppMucAdminIq::Item item;
        item.setAffiliation(affiliation);

        QXmppMucAdminIq iq;
        iq.setTo(roomJid);
        iq.setItems(QList<QXmppMucAdminIq::Item>() << item);
        if (!client()->sendPacket(iq))
            return false;
    }
    return true;
}

/// Sets the subject for the given room.
///
/// \param roomJid
/// \param subject
///
/// \return true if the request was sent, false otherwise
///

bool QXmppMucManager::setRoomSubject(const QString &roomJid, const QString &subject)
{
    QXmppMessage msg;
    msg.setTo(roomJid);
    msg.setType(QXmppMessage::GroupChat);
    msg.setSubject(subject);
    return  client()->sendPacket(msg);
}

/// Invite a user to a chat room.
///
/// \param roomJid
/// \param jid
/// \param reason
///
/// \return true if the message was sent, false otherwise
///

bool QXmppMucManager::sendInvitation(const QString &roomJid, const QString &jid, const QString &reason)
{
    QXmppElement x;
    x.setTagName("x");
    x.setAttribute("xmlns", ns_conference);
    x.setAttribute("jid", roomJid);
    x.setAttribute("reason", reason);

    QXmppMessage message;
    message.setTo(jid);
    message.setType(QXmppMessage::Normal);
    message.setExtensions(x);
    return client()->sendPacket(message);
}

/// Send a message to a chat room.
///
/// \param roomJid
/// \param text
///
/// \return true if the message was sent, false otherwise
///

bool QXmppMucManager::sendMessage(const QString &roomJid, const QString &text)
{
    if (!m_nickNames.contains(roomJid))
    {
        qWarning("Cannot send message to unknown chat room");
        return false;
    }
    QXmppMessage msg;
    msg.setBody(text);
    msg.setTo(roomJid);
    msg.setType(QXmppMessage::GroupChat);
    return client()->sendPacket(msg);
}

void QXmppMucManager::messageReceived(const QXmppMessage &msg)
{
    if (msg.type() != QXmppMessage::Normal)
        return;

    // process room invitations
    foreach (const QXmppElement &extension, msg.extensions())
    {
        if (extension.tagName() == "x" && extension.attribute("xmlns") == ns_conference)
        {
            const QString roomJid = extension.attribute("jid");
            if (!roomJid.isEmpty() && !m_nickNames.contains(roomJid))
                emit invitationReceived(roomJid, msg.from(), extension.attribute("reason"));
            break;
        }
    }
}

void QXmppMucManager::mucAdminIqReceived(const QXmppMucAdminIq &iq)
{
    if (iq.type() != QXmppIq::Result) 
        return;
    emit roomPermissionsReceived(iq.from(), iq.items());
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

