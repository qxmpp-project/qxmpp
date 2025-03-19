// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPROSTERMANAGER_H
#define QXMPPROSTERMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppPresence.h"
#include "QXmppRosterIq.h"
#include "QXmppSendResult.h"

#include <variant>

#include <QMap>
#include <QObject>
#include <QStringList>

template<typename T>
class QXmppTask;
class QXmppRosterManagerPrivate;

///
/// \brief The QXmppRosterManager class provides access to a connected client's
/// roster.
///
/// \note Its object should not be created using its constructor. Instead
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
/// \anchor rostermanager_moved
/// ## XEP-0283: Moved
///
/// \xep{0283, Moved} provides a way to inform other users that an account has moved. This allows a
/// presence subscription request from a new account to be automatically linked to the old account.
/// However, this requires verification via a corresponding PubSub node on the old account.
///
/// The roster manager automatically checks entries using the QXmppMovedManager. For this to work,
/// the moved manager must be added to the QXmppClient. If the moved manager is not found or the
/// specified old JID is incorrect, the entry in the presence subscription request will be removed.
///
/// The received and verified presence subscription request is emitted via
/// subscriptionRequestReceived() and the old JID can be found in QXmppPresence::oldJid().
/// It is safe to assume that this old JID is valid since QXmpp 1.10.2.
///
/// Accepting moved subscription requests automatically is discouraged in
/// [section 4.3](https://xmpp.org/extensions/xep-0283.html#receive-notification-client) of the
/// XEP.
///
/// ### Behaviour in previous versions
///
/// Support in the roster manager and in QXmppPresence was initially added in 1.9.0.
///
/// In QXmpp 1.9.0 until 1.9.4 and 1.10.0 until 1.10.1 subscription requests with a Moved element
/// are verified and automatically accepted without emitting subscriptionRequestReceived(). If
/// verification fails, the **invalid** old JID is passed in subscriptionRequestReceived()!
///
/// \ingroup Managers
///
class QXMPP_EXPORT QXmppRosterManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    /// Empty result containing QXmpp::Success or a QXmppError
    using Result = std::variant<QXmpp::Success, QXmppError>;

    explicit QXmppRosterManager(QXmppClient *stream);
    ~QXmppRosterManager() override;

    bool isRosterReceived() const;
    QStringList getRosterBareJids() const;
    QXmppRosterIq::Item getRosterEntry(const QString &bareJid) const;

    QStringList getResources(const QString &bareJid) const;
    QMap<QString, QXmppPresence> getAllPresencesForBareJid(
        const QString &bareJid) const;
    QXmppPresence getPresence(const QString &bareJid,
                              const QString &resource) const;

    QXmppTask<Result> addRosterItem(const QString &bareJid, const QString &name = {}, const QSet<QString> &groups = {});
    QXmppTask<Result> removeRosterItem(const QString &bareJid);
    QXmppTask<Result> renameRosterItem(const QString &bareJid, const QString &name);
    QXmppTask<QXmpp::SendResult> subscribeTo(const QString &bareJid, const QString &reason = {});
    QXmppTask<QXmpp::SendResult> unsubscribeFrom(const QString &bareJid, const QString &reason = {});

    /// \cond
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

public Q_SLOTS:
    bool acceptSubscription(const QString &bareJid, const QString &reason = {});
    bool refuseSubscription(const QString &bareJid, const QString &reason = {});
    bool addItem(const QString &bareJid, const QString &name = {}, const QSet<QString> &groups = {});
    bool removeItem(const QString &bareJid);
    bool renameItem(const QString &bareJid, const QString &name);
    bool subscribe(const QString &bareJid, const QString &reason = {});
    bool unsubscribe(const QString &bareJid, const QString &reason = {});

Q_SIGNALS:
    /// This signal is emitted when the Roster IQ is received after a successful
    /// connection. That is the roster entries are empty before this signal is emitted.
    /// One should use getRosterBareJids() and getRosterEntry() only after
    /// this signal has been emitted.
    ///
    /// \note If the previous stream could be resumed, this signal is not
    /// emitted since QXmpp 1.4. Changes since the last connection are reported
    /// via the itemAdded(), itemChanged() and itemRemoved() signals.
    void rosterReceived();

    /// This signal is emitted when the presence of a particular bareJid and resource changes.
    void presenceChanged(const QString &bareJid, const QString &resource);

    /// This signal is emitted when a contact asks to subscribe to your presence.
    ///
    /// You can either accept the request by calling acceptSubscription() or refuse it
    /// by calling refuseSubscription().
    ///
    /// \note If you set QXmppConfiguration::autoAcceptSubscriptions() to true, this
    /// signal will not be emitted. This is only valid for non moved or verified moved subscription.
    /// If the subscription is a moved one and the roster's old-jid's subscription is not either from
    /// or both then QXmppConfiguration::autoAcceptSubscriptions() is ignored.
    void subscriptionReceived(const QString &bareJid);

    void subscriptionRequestReceived(const QString &subscriberBareJid, const QXmppPresence &presence);

    /// This signal is emitted when the roster entry of a particular bareJid is
    /// added as a result of roster push.
    void itemAdded(const QString &bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid
    /// changes as a result of roster push.
    void itemChanged(const QString &bareJid);

    /// This signal is emitted when the roster entry of a particular bareJid is
    /// removed as a result of roster push.
    void itemRemoved(const QString &bareJid);

protected:
    void onRegistered(QXmppClient *client) override;
    void onUnregistered(QXmppClient *client) override;

private Q_SLOTS:
    void _q_connected();
    void _q_disconnected();
    void _q_presenceReceived(const QXmppPresence &);

private:
    using RosterResult = std::variant<QXmppRosterIq, QXmppError>;

    void handleSubscriptionRequest(const QString &bareJid, const QXmppPresence &presence);
    QXmppTask<RosterResult> requestRoster();

    const std::unique_ptr<QXmppRosterManagerPrivate> d;

    friend class QXmppMixManager;
};

#endif  // QXMPPROSTER_H
