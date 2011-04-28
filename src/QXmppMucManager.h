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

#ifndef QXMPPMUCMANAGER_H
#define QXMPPMUCMANAGER_H

#include <QMap>

#include "QXmppClientExtension.h"
#include "QXmppMucIq.h"
#include "QXmppPresence.h"

class QXmppDataForm;
class QXmppMessage;
class QXmppMucAdminIq;
class QXmppMucOwnerIq;
class QXmppMucRoom;
class QXmppMucRoomPrivate;

/// \brief The QXmppMucManager class makes it possible to interact with
/// multi-user chat rooms as defined by XEP-0045: Multi-User Chat.
///
/// To make use of this manager, you need to instantiate it and load it into
/// the QXmppClient instance as follows:
///
/// \code
/// QXmppMucManager *manager = new QXmppMucManager;
/// client->addExtension(manager);
/// \endcode
///
/// You can then join a room as follows:
///
/// \code
/// QXmppMucRoom *room = manager->addRoom("room@conference.example.com");
/// room->setNickName("mynick");
/// room->join();
/// \endcode
///
/// \ingroup Managers

class QXmppMucManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppMucRoom *addRoom(const QString &roomJid);

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when an invitation to a chat room is received.
    void invitationReceived(const QString &roomJid, const QString &inviter, const QString &reason);

protected:
    /// \cond
    void setClient(QXmppClient* client);
    /// \endcond

private slots:
    void messageReceived(const QXmppMessage &message);

private:
    QMap<QString, QXmppMucRoom*> m_rooms;
};

/// \brief The QXmppMucRoom class represents a multi-user chat room
/// as defined by XEP-0045: Multi-User Chat.
///
/// \sa QXmppMucManager

class QXmppMucRoom : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString jid READ jid)
    Q_PROPERTY(QString nickName READ nickName WRITE setNickName)
    Q_PROPERTY(QStringList participants READ participants)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString subject READ subject WRITE setSubject NOTIFY subjectChanged)

public:

    /// This enum is used to describe chat room actions.
    enum Action {
        NoAction = 0,               ///< no action
        SubjectAction = 1,          ///< change the room's subject
        ConfigurationAction = 2,    ///< change the room's configuration
        PermissionsAction = 4,      ///< change the room's permissions
        KickAction = 8,             ///< kick users from the room
    };
    Q_DECLARE_FLAGS(Actions, Action)

    ~QXmppMucRoom();

    Actions allowedActions() const;
    bool isJoined() const;
    QString jid() const;

    QString nickName() const;
    void setNickName(const QString &nickName);

    QXmppPresence participantPresence(const QString &jid) const;
    QStringList participants() const;

    QString password() const;
    void setPassword(const QString &password);

    QString subject() const;
    void setSubject(const QString &subject);

    QXmppPresence::Status status() const;
    void setStatus(const QXmppPresence::Status &status);

signals:
    /// This signal is emitted when the configuration form for the room is received.
    void configurationReceived(const QXmppDataForm &configuration);

    /// This signal is emitted once you have joined the room.
    void joined();

    /// This signal is emitted if you get kicked from the room.
    void kicked(const QString &reason);

    /// This signal is emiited once you have left the room.
    void left();

    /// This signal is emitted when a message is received.
    void messageReceived(const QXmppMessage &message);

    /// This signal is emitted when a participant joins the room.
    void participantAdded(const QString &jid);

    /// This signal is emitted when a participant changes.
    void participantChanged(const QString &jid);

    /// This signal is emitted when a participant leaves the room.
    void participantRemoved(const QString &jid);

    /// This signal is emitted when the room's permissions are received.
    void permissionsReceived(const QList<QXmppMucAdminIq::Item> &permissions);

    /// This signal is emitted when the room's subject changes.
    void subjectChanged(const QString &subject);

public slots:
    bool join();
    bool leave();
    bool requestConfiguration();
    bool requestPermissions();
    bool setConfiguration(const QXmppDataForm &form);
    bool setPermissions(const QList<QXmppMucAdminIq::Item> &permissions);
    bool sendInvitation(const QString &jid, const QString &reason);
    bool sendMessage(const QString &text);

private slots:
    void _q_disconnected();
    void _q_messageReceived(const QXmppMessage &message);
    void _q_presenceReceived(const QXmppPresence &presence);

private:
    QXmppMucRoom(QXmppClient *client, const QString &jid, QObject *parent);
    QXmppMucRoomPrivate *d;
    friend class QXmppMucManager;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppMucRoom::Actions)

#endif
