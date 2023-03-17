// SPDX-FileCopyrightText: 2023 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "QXmppClientExtension.h"
#include "QXmppError.h"
#include "QXmppTask.h"

#include <QVector>

#include <variant>

struct QXmppBlockingManagerPrivate;

class QXMPP_EXPORT QXmppBlocklist
{
public:
    struct NotBlocked
    {
    };
    struct Blocked
    {
        QVector<QString> blockingEntries;
        QVector<QString> partiallyBlockingEntries;
    };
    struct PartiallyBlocked
    {
        QVector<QString> partiallyBlockingEntries;
    };

    using BlockingState = std::variant<NotBlocked, PartiallyBlocked, Blocked>;

    QXmppBlocklist();
    QXmppBlocklist(QVector<QString> entries);
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppBlocklist)

    QVector<QString> entries() const;
    bool containsEntry(QStringView) const;
    BlockingState blockingState(const QString &jid) const;

private:
    QVector<QString> m_blocklist;
};

class QXMPP_EXPORT QXmppBlockingManager : public QXmppClientExtension
{
    Q_OBJECT

    /// Whether the blocking manager is currently receiving updates of the blocklist.
    Q_PROPERTY(bool subscribed READ isSubscribed NOTIFY subscribedChanged)

public:
    using BlocklistResult = std::variant<QXmppBlocklist, QXmppError>;
    using Result = std::variant<QXmpp::Success, QXmppError>;

    QXmppBlockingManager();
    ~QXmppBlockingManager() override;

    bool isSubscribed() const;
    Q_SIGNAL void subscribedChanged();

    QXmppTask<BlocklistResult> fetchBlocklist();
    QXmppTask<Result> block(QString jid) { return block(QVector<QString> { std::move(jid) }); }
    QXmppTask<Result> block(QVector<QString> jids);
    QXmppTask<Result> unblock(QString jid) { return unblock(QVector<QString> { std::move(jid) }); }
    QXmppTask<Result> unblock(QVector<QString> jids);

    Q_SIGNAL void blocked(const QVector<QString> &jids);
    Q_SIGNAL void unblocked(const QVector<QString> &jids);

    /// \cond
    QStringList discoveryFeatures() const override;
    void setClient(QXmppClient *) override;
    bool handleStanza(const QDomElement &, const std::optional<QXmppE2eeMetadata> &) override;
    /// \endcond

private:
    void onConnected();

    std::unique_ptr<QXmppBlockingManagerPrivate> d;
};
