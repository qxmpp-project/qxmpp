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
    using Item = QXmppTuneItem;
    using GetResult = std::variant<Item, QXmppStanza::Error>;
    using PublishResult = std::variant<QString, QXmppStanza::Error>;

    QXmppUserTuneManager();

    QStringList discoveryFeatures() const override;

    QFuture<GetResult> request(const QString &jid);
    QFuture<PublishResult> publish(const QXmppTuneItem &);

    Q_SIGNAL void itemReceived(const QString &jid, const QXmppTuneItem &item);

protected:
    /// \cond
    bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) override;
    /// \endcond
};

#endif  // QXMPPUSERTUNEMANAGER_H
