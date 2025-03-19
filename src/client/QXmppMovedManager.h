// SPDX-FileCopyrightText: 2024 Filipe Azevedo <pasnox@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMOVEDMANAGER_H
#define QXMPPMOVEDMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppSendResult.h"
#include "QXmppTask.h"

class QXmppPresence;
struct QXmppError;
class QXmppMovedManagerPrivate;

class QXMPP_EXPORT QXmppMovedManager : public QXmppClientExtension
{
    Q_OBJECT
    Q_PROPERTY(bool supportedByServer READ supportedByServer NOTIFY supportedByServerChanged)

public:
    using Result = std::variant<QXmpp::Success, QXmppError>;

    explicit QXmppMovedManager();
    ~QXmppMovedManager() override;

    QStringList discoveryFeatures() const override;

    bool supportedByServer() const;
    Q_SIGNAL void supportedByServerChanged();

    QXmppTask<Result> publishStatement(const QString &newBareJid);
    QXmppTask<Result> verifyStatement(const QString &oldBareJid, const QString &newBareJid);

    QXmppTask<QXmpp::SendResult> notifyContact(const QString &contactBareJid, const QString &oldBareJid, bool sensitive = true, const QString &reason = {});

protected:
    /// \cond
    void onRegistered(QXmppClient *client) override;
    void onUnregistered(QXmppClient *client) override;
    /// \endcond

private:
    QXmppTask<QXmppPresence> processSubscriptionRequest(QXmppPresence presence);
    void handleDiscoInfo(const QXmppDiscoveryIq &iq);
    Result movedJidsMatch(const QString &newBareJid, const QString &pepBareJid) const;

    void setSupportedByServer(bool supportedByServer);
    void resetCachedData();

    const std::unique_ptr<QXmppMovedManagerPrivate> d;

    friend class QXmppRosterManager;
    friend class tst_QXmppMovedManager;
};

#endif  // QXMPPMOVEDMANAGER_H
