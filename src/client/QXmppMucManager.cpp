/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
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

#include <QDomElement>
#include <QMap>

#include "QXmppClient.h"
#include "QXmppConstants.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppMessage.h"
#include "QXmppMucIq.h"
#include "QXmppMucManager.h"
#include "QXmppUtils.h"

class QXmppMucManagerPrivate
{
public:
    QMap<QString, QXmppMucRoom*> rooms;
};

class QXmppMucRoomPrivate
{
public:
    QString ownJid() const { return jid + "/" + nickName; }
    QXmppClient *client;
    QXmppDiscoveryManager *discoManager;
    QXmppMucRoom::Actions allowedActions;
    QString jid;
    QString name;
    QMap<QString, QXmppPresence> participants;
    QString password;
    QMap<QString, QXmppMucItem> permissions;
    QSet<QString> permissionsQueue;
    QString nickName;
    QString subject;
};

/// Constructs a new QXmppMucManager.

QXmppMucManager::QXmppMucManager()
{
    d = new QXmppMucManagerPrivate;
}

/// Destroys a QXmppMucManager.

QXmppMucManager::~QXmppMucManager()
{
    delete d;
}

/// Adds the given chat room to the set of managed rooms.
///
/// \param roomJid

QXmppMucRoom *QXmppMucManager::addRoom(const QString &roomJid)
{
    QXmppMucRoom *room = d->rooms.value(roomJid);
    if (!room) {
        room = new QXmppMucRoom(client(), roomJid, this);
        d->rooms.insert(roomJid, room);
        connect(room, SIGNAL(destroyed(QObject*)),
            this, SLOT(_q_roomDestroyed(QObject*)));

        // emit signal
        emit roomAdded(room);
    }
    return room;
}

/// Returns the list of managed rooms.

QList<QXmppMucRoom*> QXmppMucManager::rooms() const
{
    return d->rooms.values();
}

/// \cond
QStringList QXmppMucManager::discoveryFeatures() const
{
    // XEP-0045: Multi-User Chat
    return QStringList()
        << ns_muc
        << ns_muc_admin
        << ns_muc_owner
        << ns_muc_user
        << ns_conference;
}

bool QXmppMucManager::handleStanza(const QDomElement &element)
{
    if (element.tagName() == "iq")
    {
        if (QXmppMucAdminIq::isMucAdminIq(element))
        {
            QXmppMucAdminIq iq;
            iq.parse(element);

            QXmppMucRoom *room = d->rooms.value(iq.from());
            if (room && iq.type() == QXmppIq::Result && room->d->permissionsQueue.remove(iq.id())) {
                foreach (const QXmppMucItem &item, iq.items()) {
                    const QString jid = item.jid();
                    if (!room->d->permissions.contains(jid))
                        room->d->permissions.insert(jid, item);
                }
                if (room->d->permissionsQueue.isEmpty()) {
                    emit room->permissionsReceived(room->d->permissions.values());
                }
            }
            return true;
        }
        else if (QXmppMucOwnerIq::isMucOwnerIq(element))
        {
            QXmppMucOwnerIq iq;
            iq.parse(element);

            QXmppMucRoom *room = d->rooms.value(iq.from());
            if (room && iq.type() == QXmppIq::Result && !iq.form().isNull())
                emit room->configurationReceived(iq.form());
            return true;
        }
    }
    return false;
}

void QXmppMucManager::setClient(QXmppClient* client)
{
    bool check;
    Q_UNUSED(check);

    QXmppClientExtension::setClient(client);

    check = connect(client, SIGNAL(messageReceived(QXmppMessage)),
                    this, SLOT(_q_messageReceived(QXmppMessage)));
    Q_ASSERT(check);
}
/// \endcond

void QXmppMucManager::_q_messageReceived(const QXmppMessage &msg)
{
    if (msg.type() != QXmppMessage::Normal)
        return;

    // process room invitations
    const QString roomJid = msg.mucInvitationJid();
    if (!roomJid.isEmpty() && (!d->rooms.contains(roomJid) || !d->rooms.value(roomJid)->isJoined())) {
        emit invitationReceived(roomJid, msg.from(), msg.mucInvitationReason());
    }
}

void QXmppMucManager::_q_roomDestroyed(QObject *object)
{
    const QString key = d->rooms.key(static_cast<QXmppMucRoom*>(object));
    d->rooms.remove(key);
}

/// Constructs a new QXmppMucRoom.
///
/// \param parent

QXmppMucRoom::QXmppMucRoom(QXmppClient *client, const QString &jid, QObject *parent)
    : QObject(parent)
{
    bool check;
    Q_UNUSED(check);

    d = new QXmppMucRoomPrivate;
    d->allowedActions = NoAction;
    d->client = client;
    d->discoManager = client->findExtension<QXmppDiscoveryManager>();
    d->jid = jid;

    check = connect(d->client, SIGNAL(disconnected()),
                    this, SLOT(_q_disconnected()));
    Q_ASSERT(check);

    check = connect(d->client, SIGNAL(messageReceived(QXmppMessage)),
                    this, SLOT(_q_messageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = connect(d->client, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SLOT(_q_presenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    if (d->discoManager) {
        check = connect(d->discoManager, SIGNAL(infoReceived(QXmppDiscoveryIq)),
                        this, SLOT(_q_discoveryInfoReceived(QXmppDiscoveryIq)));
        Q_ASSERT(check);
    }

    // convenience signals for properties
    check = connect(this, SIGNAL(joined()), this, SIGNAL(isJoinedChanged()));
    Q_ASSERT(check);

    check = connect(this, SIGNAL(left()), this, SIGNAL(isJoinedChanged()));
    Q_ASSERT(check);
}

/// Destroys a QXmppMucRoom.

QXmppMucRoom::~QXmppMucRoom()
{
    delete d;
}

/// Returns the actions you are allowed to perform on the room.

QXmppMucRoom::Actions QXmppMucRoom::allowedActions() const
{
    return d->allowedActions;
}

/// Bans the specified user from the chat room.
///
/// The specified \a jid is the Bare JID of the form "user@host".
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::ban(const QString &jid, const QString &reason)
{
    if (!QXmppUtils::jidToResource(jid).isEmpty()) {
        qWarning("QXmppMucRoom::ban expects a bare JID");
        return false;
    }

    QXmppMucItem item;
    item.setAffiliation(QXmppMucItem::OutcastAffiliation);
    item.setJid(jid);
    item.setReason(reason);

    QXmppMucAdminIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(d->jid);
    iq.setItems(QList<QXmppMucItem>() << item);

    return d->client->sendPacket(iq);
}

/// Returns true if you are currently in the room.

bool QXmppMucRoom::isJoined() const
{
    return d->participants.contains(d->ownJid());
}

/// Returns the chat room's bare JID.

QString QXmppMucRoom::jid() const
{
    return d->jid;
}

/// Joins the chat room.
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::join()
{
    if (isJoined() || d->nickName.isEmpty())
        return false;

    // reflect our current presence in the chat room
    QXmppPresence packet = d->client->clientPresence();
    packet.setTo(d->ownJid());
    packet.setType(QXmppPresence::Available);
    packet.setMucPassword(d->password);
    packet.setMucSupported(true);
    return d->client->sendPacket(packet);
}

/// Kicks the specified user from the chat room.
///
/// The specified \a jid is the Occupant JID of the form "room@service/nick".
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::kick(const QString &jid, const QString &reason)
{
    QXmppMucItem item;
    item.setNick(QXmppUtils::jidToResource(jid));
    item.setRole(QXmppMucItem::NoRole);
    item.setReason(reason);

    QXmppMucAdminIq iq;
    iq.setType(QXmppIq::Set);
    iq.setTo(d->jid);
    iq.setItems(QList<QXmppMucItem>() << item);

    return d->client->sendPacket(iq);
}

/// Leaves the chat room.
///
/// \param message An optional message.
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::leave(const QString &message)
{
    QXmppPresence packet;
    packet.setTo(d->ownJid());
    packet.setType(QXmppPresence::Unavailable);
    packet.setStatusText(message);
    return d->client->sendPacket(packet);
}

/// Returns the chat room's human-readable name.
///
/// This name will only be available after the room has been joined.

QString QXmppMucRoom::name() const
{
    return d->name;
}

/// Returns your own nickname.

QString QXmppMucRoom::nickName() const
{
    return d->nickName;
}

/// Invites a user to the chat room.
///
/// \param jid
/// \param reason
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::sendInvitation(const QString &jid, const QString &reason)
{
    QXmppMessage message;
    message.setTo(jid);
    message.setType(QXmppMessage::Normal);
    message.setMucInvitationJid(d->jid);
    message.setMucInvitationReason(reason);
    return d->client->sendPacket(message);
}

/// Sends a message to the room.
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::sendMessage(const QString &text)
{
    QXmppMessage msg;
    msg.setTo(d->jid);
    msg.setType(QXmppMessage::GroupChat);
    msg.setBody(text);
    return d->client->sendPacket(msg);
}

/// Sets your own nickname.
///
/// You need to set your nickname before calling join().
///
/// \param nickName

void QXmppMucRoom::setNickName(const QString &nickName)
{
    if (nickName == d->nickName)
        return;

    // if we had already joined the room, request nickname change
    if (isJoined()) {
        QXmppPresence packet = d->client->clientPresence();
        packet.setTo(d->jid + "/" + nickName);
        packet.setType(QXmppPresence::Available);
        d->client->sendPacket(packet);
    }
    else {
        d->nickName = nickName;
        emit nickNameChanged(nickName);
    }
}

/// Returns the "Full JID" of the given participant.
///
/// The specified \a jid is the Occupant JID of the form "room@service/nick".

QString QXmppMucRoom::participantFullJid(const QString &jid) const
{
    if (d->participants.contains(jid))
        return d->participants.value(jid).mucItem().jid();
    else
        return QString();
}

/// Returns the presence for the given participant.
///
/// The specified \a jid is the Occupant JID of the form "room@service/nick".

QXmppPresence QXmppMucRoom::participantPresence(const QString &jid) const
{
    if (d->participants.contains(jid))
        return d->participants.value(jid);

    QXmppPresence presence;
    presence.setFrom(jid);
    presence.setType(QXmppPresence::Unavailable);
    return presence;
}

/// Returns the list of participant JIDs.
///
/// These JIDs are Occupant JIDs of the form "room@service/nick".

QStringList QXmppMucRoom::participants() const
{
    return d->participants.keys();
}

/// Returns the chat room password.

QString QXmppMucRoom::password() const
{
    return d->password;
}

/// Sets the chat room password.
///
/// \param password

void QXmppMucRoom::setPassword(const QString &password)
{
    d->password = password;
}

/// Returns the room's subject.

QString QXmppMucRoom::subject() const
{
    return d->subject;
}

/// Sets the chat room's subject.
///
/// \param subject

void QXmppMucRoom::setSubject(const QString &subject)
{
    QXmppMessage msg;
    msg.setTo(d->jid);
    msg.setType(QXmppMessage::GroupChat);
    msg.setSubject(subject);
    d->client->sendPacket(msg);
}

/// Request the configuration form for the chat room.
///
/// \return true if the request was sent, false otherwise
///
/// \sa configurationReceived()

bool QXmppMucRoom::requestConfiguration()
{
    QXmppMucOwnerIq iq;
    iq.setTo(d->jid);
    return d->client->sendPacket(iq);
}

/// Send the configuration form for the chat room.
///
/// \param form
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::setConfiguration(const QXmppDataForm &form)
{
    QXmppMucOwnerIq iqPacket;
    iqPacket.setType(QXmppIq::Set);
    iqPacket.setTo(d->jid);
    iqPacket.setForm(form);
    return d->client->sendPacket(iqPacket);
}

/// Request the room's permissions.
///
/// \return true if the request was sent, false otherwise
///
/// \sa permissionsReceived()

bool QXmppMucRoom::requestPermissions()
{
    QList<QXmppMucItem::Affiliation> affiliations;
    affiliations << QXmppMucItem::OwnerAffiliation;
    affiliations << QXmppMucItem::AdminAffiliation;
    affiliations << QXmppMucItem::MemberAffiliation;
    affiliations << QXmppMucItem::OutcastAffiliation;

    d->permissions.clear();
    d->permissionsQueue.clear();
    foreach (QXmppMucItem::Affiliation affiliation, affiliations) {
        QXmppMucItem item;
        item.setAffiliation(affiliation);

        QXmppMucAdminIq iq;
        iq.setTo(d->jid);
        iq.setItems(QList<QXmppMucItem>() << item);
        if (!d->client->sendPacket(iq))
            return false;
        d->permissionsQueue += iq.id();
    }
    return true;
}

/// Sets the room's permissions.
///
/// \param permissions
///
/// \return true if the request was sent, false otherwise

bool QXmppMucRoom::setPermissions(const QList<QXmppMucItem> &permissions)
{
    QList<QXmppMucItem> items;

    // Process changed members
    foreach (const QXmppMucItem &item, permissions) {
        const QString jid = item.jid();
        if (d->permissions.value(jid).affiliation() != item.affiliation())
            items << item;
        d->permissions.remove(jid);
    }

    // Process deleted members
    foreach (const QString &jid, d->permissions.keys()) {
        QXmppMucItem item;
        item.setAffiliation(QXmppMucItem::NoAffiliation);
        item.setJid(jid);
        items << item;
        d->permissions.remove(jid);
    }

    // Don't send request if there are no changes
    if (items.isEmpty())
        return false;

    QXmppMucAdminIq iq;
    iq.setTo(d->jid);
    iq.setType(QXmppIq::Set);
    iq.setItems(items);
    return d->client->sendPacket(iq);
}

void QXmppMucRoom::_q_disconnected()
{
    const bool wasJoined = isJoined();

    // clear chat room participants
    const QStringList removed = d->participants.keys();
    d->participants.clear();
    foreach (const QString &jid, removed)
        emit participantRemoved(jid);
    emit participantsChanged();

    // update available actions
    if (d->allowedActions != NoAction) {
        d->allowedActions = NoAction;
        emit allowedActionsChanged(d->allowedActions);
    }

    // emit "left" signal if we had joined the room
    if (wasJoined)
        emit left();
}

void QXmppMucRoom::_q_discoveryInfoReceived(const QXmppDiscoveryIq &iq)
{
    if (iq.from() == d->jid) {
        QString name;
        foreach (const QXmppDiscoveryIq::Identity &identity, iq.identities()) {
            if (identity.category() == "conference") {
                name = identity.name();
                break;
            }
        }

        if (name != d->name) {
            d->name = name;
            emit nameChanged(name);
        }
    }
}

void QXmppMucRoom::_q_messageReceived(const QXmppMessage &message)
{
    if (QXmppUtils::jidToBareJid(message.from())!= d->jid)
        return;

    // handle message subject
    const QString subject = message.subject();
    if (!subject.isEmpty()) {
        d->subject = subject;
        emit subjectChanged(subject);
    }

    emit messageReceived(message);
}

void QXmppMucRoom::_q_presenceReceived(const QXmppPresence &presence)
{
    const QString jid = presence.from();

    // if our own presence changes, reflect it in the chat room
    if (isJoined() && jid == d->client->configuration().jid()) {
        QXmppPresence packet = d->client->clientPresence();
        packet.setTo(d->ownJid());
        d->client->sendPacket(packet);
    }

    if (QXmppUtils::jidToBareJid(jid) != d->jid)
        return;

    if (presence.type() == QXmppPresence::Available) {
        const bool added = !d->participants.contains(jid);
        d->participants.insert(jid, presence);

        // refresh allowed actions
        if (jid == d->ownJid()) {

            QXmppMucItem mucItem = presence.mucItem();
            Actions newActions = NoAction;

            // role
            if (mucItem.role() == QXmppMucItem::ModeratorRole)
                newActions |= (KickAction | SubjectAction);

            // affiliation
            if (mucItem.affiliation() == QXmppMucItem::OwnerAffiliation)
                newActions |= (ConfigurationAction | PermissionsAction | SubjectAction);
            else if (mucItem.affiliation() == QXmppMucItem::AdminAffiliation)
                newActions |= (PermissionsAction | SubjectAction);

            if (newActions != d->allowedActions) {
                d->allowedActions = newActions;
                emit allowedActionsChanged(d->allowedActions);
            }
        }

        if (added) {
            emit participantAdded(jid);
            emit participantsChanged();
            if (jid == d->ownJid()) {
                // request room information
                if (d->discoManager)
                    d->discoManager->requestInfo(d->jid);

                emit joined();
            }
        } else {
            emit participantChanged(jid);
        }
    }
    else if (presence.type() == QXmppPresence::Unavailable) {
        if (d->participants.contains(jid)) {
            d->participants.insert(jid, presence);

            emit participantRemoved(jid);
            d->participants.remove(jid);
            emit participantsChanged();

            // check whether this was our own presence
            if (jid == d->ownJid()) {
                const QString newNick = presence.mucItem().nick();
                if (!newNick.isEmpty() && newNick != d->nickName) {
                    d->nickName = newNick;
                    emit nickNameChanged(newNick);
                    return;
                }

                // check whether we were kicked
                if (presence.mucStatusCodes().contains(307)) {
                    const QString actor = presence.mucItem().actor();
                    const QString reason = presence.mucItem().reason();
                    emit kicked(actor, reason);
                }

                // clear chat room participants
                const QStringList removed = d->participants.keys();
                d->participants.clear();
                foreach (const QString &jid, removed)
                    emit participantRemoved(jid);
                emit participantsChanged();

                // update available actions
                if (d->allowedActions != NoAction) {
                    d->allowedActions = NoAction;
                    emit allowedActionsChanged(d->allowedActions);
                }

                // notify user we left the room
                emit left();
            }
        }
    }
    else if (presence.type() == QXmppPresence::Error) {
        if (presence.isMucSupported()) {
            // emit error
            emit error(presence.error());

            // notify the user we left the room
            emit left();
        }
   }
}
