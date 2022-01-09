/*
 * Copyright (C) 2008-2022 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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

#ifndef QXMPPENTITYTIMEMANAGER_H
#define QXMPPENTITYTIMEMANAGER_H

#include "QXmppClientExtension.h"

#include <variant>

template<class T>
class QFuture;
class QXmppEntityTimeIq;

///
/// \brief The QXmppEntityTimeManager class provided the functionality to get
/// the local time of an entity as defined by \xep{0202}: Entity Time.
///
/// \ingroup Managers
///
class QXMPP_EXPORT QXmppEntityTimeManager : public QXmppClientExtension
{
    Q_OBJECT

public:
    QString requestTime(const QString &jid);

    using EntityTimeResult = std::variant<QXmppEntityTimeIq, QXmppStanza::Error>;
    QFuture<EntityTimeResult> requestEntityTime(const QString &jid);

    /// \cond
    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &element) override;
    /// \endcond

Q_SIGNALS:
    /// \brief This signal is emitted when a time response is received. It's not
    /// emitted when the QFuture-based request is used.
    void timeReceived(const QXmppEntityTimeIq &);
};

#endif  // QXMPPENTITYTIMEMANAGER_H
