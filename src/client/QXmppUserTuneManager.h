/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Linus Jahn
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
