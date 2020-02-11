/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppClientExtension.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"

#include <QMap>
#include <QObject>
#include <QStringList>

class QXmppRosterManagerPrivate;

/// \brief The QXmppRosterManager class provides access to a connected client's
/// roster.
///
/// \note It's object should not be created using it's constructor. Instead
/// \c QXmppClient::findExtension<QXmppRosterManager>() should be used to get
/// the instantiated object of this class.
///
/// It stores all the Roster and Presence details of all the roster entries
/// (that is all the bareJids) in the client's friend's list. It provides the
/// functionality to get all the bareJids in the client's roster and Roster and
/// Presence details of the same.
///
/// After the QXmpp connected successfully to the XMPP server the signal
/// \c QXmppClient::connected() is emitted and the roster is requested from the
/// server. Once QXmpp receives the roster the signal
/// \c QXmppRosterManager::rosterReceived() is emitted and after that the
/// methods of this class can be used to get the roster entries.
///
/// \c QXmppRosterManager::isRosterReceived() can be used to find out whether
/// the roster has been received yet.
///
/// The \c itemAdded(), \c itemChanged() and \c itemRemoved() signals are
/// emitted whenever roster entries are added, changed or removed.
///
/// The \c presenceChanged() signal is emitted whenever the presence for a
/// roster item changes.
///
/// \ingroup Managers

class QXMPP_EXPORT QXmppRosterManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QXmppRosterManager(QXmppClient *stream);
    ~QXmppRosterManager() override;

    bool isRosterReceived() const;
    QStringList getRosterBareJids() const;
    QXmppRosterIq::Item getRosterEntry(const QString &bareJid) const;

    QStringList getResources(const QString &bareJid) const;
    QMap<QString, QXmppPresence> getAllPresencesForBareJid(
        const QString &bareJid) const;
    QXmppPresence getPresence(const QString &bareJid,
                              const QString &resource) const;

    /// \cond
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

public Q_SLOTS:
    bool acceptSubscription(const QString &bareJid, const QString &reason = QString());
    bool refuseSubscription(const QString &bareJid, const QString &reason = QString());
    bool addItem(const QString &bareJid, const QString &name = QString(), const QSet<QString> &groups = QSet<QString>());
    bool removeItem(const QString &bareJid);
    bool renameItem(const QString &bareJid, const QString &name);
    bool subscribe(const QString &bareJid, const QString &reason = QString());
    bool unsubscribe(const QString &bareJid, const QString &reason = QString());

Q_SIGNALS:
    /// This signal is emitted when the Roster IQ is received after a successful
    /// connection. That is the roster entries are empty before this signal is emitted.
    /// One should use getRosterBareJids() and getRosterEntry() only after
    /// this signal has been emitted.
    void rosterReceived();

    /// This signal is emitted when the presence of a particular bareJid and resource changes.
    void presenceChanged(const QString &bareJid, const QString &resource);

    /// This signal is emitted when a contact asks to subscribe to your presence.
    ///
    /// You can either accept the request by calling acceptSubscription() or refuse it
    /// by calling refuseSubscription().
    ///
    /// \note If you set QXmppConfiguration::autoAcceptSubscriptions() to true, this
    /// signal will not be emitted.
    void subscriptionReceived(const QString &bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid is
    /// added as a result of roster push.
    void itemAdded(const QString &bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid
    /// changes as a result of roster push.
    void itemChanged(const QString &bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid is
    /// removed as a result of roster push.
    void itemRemoved(const QString &bareJid);

private Q_SLOTS:
    void _q_connected();
    void _q_disconnected();
    void _q_presenceReceived(const QXmppPresence &);

private:
    QXmppRosterManagerPrivate *d;
};

#endif  // QXMPPROSTER_H
