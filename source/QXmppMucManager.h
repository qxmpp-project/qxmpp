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

#ifndef QXMPPMUCMANAGER_H
#define QXMPPMUCMANAGER_H

#include <QObject>
#include <QMap>

#include "QXmppPresence.h"

class QXmppDataForm;
class QXmppMessage;
class QXmppMucAdminIq;
class QXmppMucOwnerIq;
class QXmppStream;

/// \brief The QXmppMucManager class makes it possible to interact with
/// multi-user chat rooms as defined by XEP-0045: Multi-User Chat.
///
/// \ingroup Managers

class QXmppMucManager : public QObject
{
    Q_OBJECT

public:
    QXmppMucManager(QXmppStream* stream, QObject *parent = 0);

    bool joinRoom(const QString &roomJid, const QString &nickName);
    bool leaveRoom(const QString &roomJid);

    bool requestRoomConfiguration(const QString &roomJid);
    bool setRoomConfiguration(const QString &roomJid, const QXmppDataForm &form);

    bool sendInvitation(const QString &roomJid, const QString &jid, const QString &reason);
    bool sendMessage(const QString &roomJid, const QString &text);

    QMap<QString, QXmppPresence> roomParticipants(const QString& bareJid) const;

signals:
    /// This signal is emitted when an invitation to a chat room is received.
    void invitationReceived(const QString &roomJid, const QString &inviter, const QString &reason);

    /// This signal is emitted when the configuration form for a chat room is received.
    void roomConfigurationReceived(const QString &roomJid, const QXmppDataForm &configuration);

    void roomParticipantChanged(const QString &roomJid, const QString &nickName);

private slots:
    void messageReceived(const QXmppMessage &message);
    void mucAdminIqReceived(const QXmppMucAdminIq &iq);
    void mucOwnerIqReceived(const QXmppMucOwnerIq &iq);
    void presenceReceived(const QXmppPresence &presence);

private:
    QXmppStream *m_stream;
    QMap<QString, QString> m_nickNames;
    QMap<QString, QMap<QString, QXmppPresence> > m_participants;
};

#endif
