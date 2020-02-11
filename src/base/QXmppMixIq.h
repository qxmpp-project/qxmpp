/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPMIXIQ_H
#define QXMPPMIXIQ_H

#include "QXmppIq.h"

#include <QSharedDataPointer>

class QXmppMixIqPrivate;

/// \brief The QXmppMixIq class represents an IQ used to do actions on a MIX
/// channel as defined by \xep{0369}: Mediated Information eXchange (MIX) and
/// \xep{0405}: Mediated Information eXchange (MIX): Participant Server
/// Requirements.
///
/// \since QXmpp 1.1
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMixIq : public QXmppIq
{
public:
    enum Type {
        None,
        ClientJoin,
        ClientLeave,
        Join,
        Leave,
        UpdateSubscription,
        SetNick,
        Create,
        Destroy
    };

    QXmppMixIq();
    QXmppMixIq(const QXmppMixIq &);
    ~QXmppMixIq() override;

    QXmppMixIq &operator=(const QXmppMixIq &);

    QXmppMixIq::Type actionType() const;
    void setActionType(QXmppMixIq::Type);

    QString jid() const;
    void setJid(const QString &);

    QString channelName() const;
    void setChannelName(const QString &);

    QStringList nodes() const;
    void setNodes(const QStringList &);

    QString nick() const;
    void setNick(const QString &);

    /// \cond
    static bool isMixIq(const QDomElement &);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppMixIqPrivate> d;
};

#endif  // QXMPPMIXIQ_H
