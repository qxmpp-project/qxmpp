// SPDX-FileCopyrightText: 2021 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPUSERTUNEMANAGER_H
#define QXMPPUSERTUNEMANAGER_H

#include "QXmppPubSubEventManager.h"

#include <variant>

class QXmppTuneItem;

class QXMPP_EXPORT QXmppUserTuneManager : public QXmppPubSubEventManager
{
    Q_OBJECT

public:
    using TuneResult = std::variant<QXmppTuneItem, QXmppStanza::Error>;
    using PublishResult = std::variant<QString, QXmppStanza::Error>;

    QXmppUserTuneManager();

    QStringList discoveryFeatures() const override;

    QFuture<TuneResult> request(const QString &jid);
    QFuture<PublishResult> publish(const QXmppTuneItem &);

    Q_SIGNAL void userTuneChanged(const QString &jid, const QXmppTuneItem &);

protected:
    /// \cond
    bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) override;
    /// \endcond
};

#endif  // QXMPPUSERTUNEMANAGER_H
