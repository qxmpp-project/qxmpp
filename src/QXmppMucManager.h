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
/// \ingroup Managers

class QXmppMucManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    bool joinRoom(const QString &roomJid, const QString &nickName, const QString &password = QString());
    bool leaveRoom(const QString &roomJid);

    bool requestRoomConfiguration(const QString &roomJid);
    bool setRoomConfiguration(const QString &roomJid, const QXmppDataForm &form);

    bool requestRoomPermissions(const QString &roomJid);

    bool setRoomSubject(const QString &roomJid, const QString &subject);

    bool sendInvitation(const QString &roomJid, const QString &jid, const QString &reason);
    bool sendMessage(const QString &roomJid, const QString &text);

    QMap<QString, QXmppPresence> roomParticipants(const QString& bareJid) const;

    /// \cond
    QStringList discoveryFeatures() const;
    bool handleStanza(const QDomElement &element);
    /// \endcond

signals:
    /// This signal is emitted when an invitation to a chat room is received.
    void invitationReceived(const QString &roomJid, const QString &inviter, const QString &reason);

    /// This signal is emitted when the configuration form for a chat room is received.
    void roomConfigurationReceived(const QString &roomJid, const QXmppDataForm &configuration);

    /// This signal is emitted when the permissions for a chat room are received.
    void roomPermissionsReceived(const QString &roomJid, const QList<QXmppMucAdminIq::Item> &permissions);

    /// This signal is emitted when a room participant's presence changed.
    ///
    /// \sa roomParticipants()
    void roomParticipantChanged(const QString &roomJid, const QString &nickName);

protected:
    /// \cond
    void setClient(QXmppClient* client);
    /// \endcond

private slots:
    void messageReceived(const QXmppMessage &message);
    void mucAdminIqReceived(const QXmppMucAdminIq &iq);
    void mucOwnerIqReceived(const QXmppMucOwnerIq &iq);
    void presenceReceived(const QXmppPresence &presence);

private:
    QMap<QString, QString> m_nickNames;
    QMap<QString, QMap<QString, QXmppPresence> > m_participants;
};

#endif
