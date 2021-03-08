/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin
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

#ifndef QXMPPMIXINVITATION_H
#define QXMPPMIXINVITATION_H

#include "QXmppElement.h"

#include <QSharedDataPointer>

class QXmppMixInvitationPrivate;

///
/// \brief The QXmppMixInvitation class is used to invite a user to a
/// \xep{0369}: Mediated Information eXchange (MIX) channel as defined by
/// \xep{0407}: Mediated Information eXchange (MIX): Miscellaneous Capabilities.
///
/// \ingroup Stanzas
///
/// \since QXmpp 1.4
///
class QXMPP_EXPORT QXmppMixInvitation
{
public:
    QXmppMixInvitation();
    QXmppMixInvitation(const QXmppMixInvitation &other);
    ~QXmppMixInvitation();

    QXmppMixInvitation &operator=(const QXmppMixInvitation &other);

    QString inviterJid() const;
    void setInviterJid(const QString &inviterJid);

    QString inviteeJid() const;
    void setInviteeJid(const QString &inviteeJid);

    QString channelJid() const;
    void setChannelJid(const QString &channelJid);

    QString token() const;
    void setToken(const QString &token);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isMixInvitation(const QDomElement &element);

private:
    QSharedDataPointer<QXmppMixInvitationPrivate> d;
};

#endif  // QXMPPMIXINVITATION_H
