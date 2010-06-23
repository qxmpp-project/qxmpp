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
class QXmppMucAdminIq;
class QXmppMucOwnerIq;
class QXmppStream;

/// \brief The QXmppMucManager class makes it possible to interact with
/// multi-user chat rooms as defined by XEP-0045: Multi-User Chat.
///

class QXmppMucManager : public QObject
{
    Q_OBJECT

public:
    QXmppMucManager(QXmppStream* stream, QObject *parent = 0);

    bool joinRoom(const QString &bareJid, const QString &nickName);
    bool leaveRoom(const QString &bareJid);

    bool requestRoomConfiguration(const QString &bareJid);
    bool setRoomConfiguration(const QString &bareJid, const QXmppDataForm &form);

    bool sendMessage(const QString &bareJid, const QString &text);

    QMap<QString, QXmppPresence> roomParticipants(const QString& bareJid) const;

signals:
    void roomConfigurationReceived(const QString &bareJid, const QXmppDataForm &configuration);
    void roomParticipantChanged(const QString &bareJid, const QString &nickName);

private slots:
    void mucAdminIqReceived(const QXmppMucAdminIq &iq);
    void mucOwnerIqReceived(const QXmppMucOwnerIq &iq);
    void presenceReceived(const QXmppPresence &presence);

private:
    QXmppStream *m_stream;
    QMap<QString, QString> m_nickNames;
    QMap<QString, QMap<QString, QXmppPresence> > m_participants;
};

#endif
