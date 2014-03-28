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

#ifndef QXMPPMUCMANAGER_H
#define QXMPPMUCMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppMucIq.h"
#include "QXmppPresence.h"

class QXmppDataForm;
class QXmppDiscoveryIq;
class QXmppMessage;
class QXmppMucManagerPrivate;
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

class QXMPP_EXPORT QXmppMucManager : public QXmppClientExtension
{
    Q_OBJECT
    Q_PROPERTY(QList<QXmppMucRoom*> rooms READ rooms NOTIFY roomAdded)

public:
    QXmppMucManager();
    ~QXmppMucManager();

    QXmppMucRoom *addRoom(const QString &roomJid);
    QList<QXmppMucRoom*> rooms() const;

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when an invitation to a chat room is received.
    void invitationReceived(const QString &roomJid, const QString &inviter, const QString &reason);

    /// This signal is emitted when a new room is managed.
    void roomAdded(QXmppMucRoom *room);

protected:
    /// \cond
    void setClient(QXmppClient* client);
    /// \endcond

private slots:
    void _q_messageReceived(const QXmppMessage &message);
    void _q_roomDestroyed(QObject *object);

private:
    QXmppMucManagerPrivate *d;
};

/// \brief The QXmppMucRoom class represents a multi-user chat room
/// as defined by XEP-0045: Multi-User Chat.
///
/// \sa QXmppMucManager

class QXMPP_EXPORT QXmppMucRoom : public QObject
{
    Q_OBJECT
    Q_FLAGS(Action Actions)
    Q_PROPERTY(QXmppMucRoom::Actions allowedActions READ allowedActions NOTIFY allowedActionsChanged)
    Q_PROPERTY(bool isJoined READ isJoined NOTIFY isJoinedChanged)
    Q_PROPERTY(QString jid READ jid CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString nickName READ nickName WRITE setNickName NOTIFY nickNameChanged)
    Q_PROPERTY(QStringList participants READ participants NOTIFY participantsChanged)
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
    QString name() const;

    QString nickName() const;
    void setNickName(const QString &nickName);

    Q_INVOKABLE QString participantFullJid(const QString &jid) const;
    QXmppPresence participantPresence(const QString &jid) const;
    QStringList participants() const;

    QString password() const;
    void setPassword(const QString &password);

    QString subject() const;
    void setSubject(const QString &subject);

signals:
    /// This signal is emitted when the allowed actions change.
    void allowedActionsChanged(QXmppMucRoom::Actions actions) const;

    /// This signal is emitted when the configuration form for the room is received.
    void configurationReceived(const QXmppDataForm &configuration);

    /// This signal is emitted when an error is encountered.
    void error(const QXmppStanza::Error &error);

    /// This signal is emitted once you have joined the room.
    void joined();

    /// This signal is emitted if you get kicked from the room.
    void kicked(const QString &jid, const QString &reason);

    /// \cond
    void isJoinedChanged();
    /// \endcond

    /// This signal is emitted once you have left the room.
    void left();

    /// This signal is emitted when a message is received.
    void messageReceived(const QXmppMessage &message);

    /// This signal is emitted when the room's human-readable name changes.
    void nameChanged(const QString &name);

    /// This signal is emitted when your own nick name changes.
    void nickNameChanged(const QString &nickName);

    /// This signal is emitted when a participant joins the room.
    void participantAdded(const QString &jid);

    /// This signal is emitted when a participant changes.
    void participantChanged(const QString &jid);

    /// This signal is emitted when a participant leaves the room.
    void participantRemoved(const QString &jid);

    /// \cond
    void participantsChanged();
    /// \endcond

    /// This signal is emitted when the room's permissions are received.
    void permissionsReceived(const QList<QXmppMucItem> &permissions);

    /// This signal is emitted when the room's subject changes.
    void subjectChanged(const QString &subject);

public slots:
    bool ban(const QString &jid, const QString &reason);
    bool join();
    bool kick(const QString &jid, const QString &reason);
    bool leave(const QString &message = QString());
    bool requestConfiguration();
    bool requestPermissions();
    bool setConfiguration(const QXmppDataForm &form);
    bool setPermissions(const QList<QXmppMucItem> &permissions);
    bool sendInvitation(const QString &jid, const QString &reason);
    bool sendMessage(const QString &text);

private slots:
    void _q_disconnected();
    void _q_discoveryInfoReceived(const QXmppDiscoveryIq &iq);
    void _q_messageReceived(const QXmppMessage &message);
    void _q_presenceReceived(const QXmppPresence &presence);

private:
    QXmppMucRoom(QXmppClient *client, const QString &jid, QObject *parent);
    QXmppMucRoomPrivate *d;
    friend class QXmppMucManager;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QXmppMucRoom::Actions)

#endif
