/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#ifndef QXMPPROSTERMANAGER_H
#define QXMPPROSTERMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>

#include "QXmppClientExtension.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"

class QXmppRosterManagerPrivate;

/// \brief The QXmppRosterManager class provides access to a connected client's roster.
///
/// \note It's object should not be created using it's constructor. Instead
/// QXmppClient::rosterManager() should be used to get the reference of instantiated
/// object this class.
///
/// It stores all the Roster and Presence details of all the roster entries (that
/// is all the bareJids) in the client's friend's list. It provides the
/// functionality to get all the bareJids in the client's roster and Roster and
/// Presence details of the same.
///
/// After the successful xmpp connection that after the signal QXmppClient::connected()
/// is emitted QXmpp requests for getting the roster. Once QXmpp receives the roster
/// the signal QXmppRosterManager::rosterReceived() is emitted and after that user can
/// use the functions of this class to get roster entries.
///
/// Function QXmppRosterManager::isRosterReceived() tells whether the roster has been
/// received or not.
///
/// The itemAdded(), itemChanged() and itemRemoved() signals are emitted whenever roster
/// entries are added, changed or removed.
///
/// The presenceChanged() signal is emitted whenever the presence for a roster item changes.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppRosterManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppRosterManager(QXmppClient* stream);
    ~QXmppRosterManager();

    bool isRosterReceived() const;
    QStringList getRosterBareJids() const;
    QXmppRosterIq::Item getRosterEntry(const QString& bareJid) const;

    QStringList getResources(const QString& bareJid) const;
    QMap<QString, QXmppPresence> getAllPresencesForBareJid(
            const QString& bareJid) const;
    QXmppPresence getPresence(const QString& bareJid,
                              const QString& resource) const;

    /// \cond
    bool handleStanza(const QDomElement &element);
    /// \endcond

public slots:
    bool acceptSubscription(const QString &bareJid, const QString &reason = QString());
    bool refuseSubscription(const QString &bareJid, const QString &reason = QString());
    bool addItem(const QString &bareJid, const QString &name = QString(), const QSet<QString> &groups = QSet<QString>());
    bool removeItem(const QString &bareJid);
    bool renameItem(const QString &bareJid, const QString &name);
    bool subscribe(const QString &bareJid, const QString &reason = QString());
    bool unsubscribe(const QString &bareJid, const QString &reason = QString());

signals:
    /// This signal is emitted when the Roster IQ is received after a successful
    /// connection. That is the roster entries are empty before this signal is emitted.
    /// One should use getRosterBareJids() and getRosterEntry() only after
    /// this signal has been emitted.
    void rosterReceived();

    /// This signal is emitted when the presence of a particular bareJid and resource changes.
    void presenceChanged(const QString& bareJid, const QString& resource);

    /// This signal is emitted when a contact asks to subscribe to your presence.
    ///
    /// You can either accept the request by calling acceptSubscription() or refuse it
    /// by calling refuseSubscription().
    ///
    /// \note If you set QXmppConfiguration::autoAcceptSubscriptions() to true, this
    /// signal will not be emitted.
    void subscriptionReceived(const QString& bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid is
    /// added as a result of roster push.
    void itemAdded(const QString& bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid
    /// changes as a result of roster push.
    void itemChanged(const QString& bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid is
    /// removed as a result of roster push.
    void itemRemoved(const QString& bareJid);

private slots:
    void _q_connected();
    void _q_disconnected();
    void _q_presenceReceived(const QXmppPresence&);

private:
    QXmppRosterManagerPrivate *d;
};

#endif // QXMPPROSTER_H
