// SPDX-FileCopyrightText: 2021 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPMIXINVITATION_H
#define QXMPPMIXINVITATION_H

#include "QXmppElement.h"

#include <QSharedDataPointer>

class QXmppMixInvitationPrivate;

class QXMPP_EXPORT QXmppMixInvitation
{
public:
    QXmppMixInvitation();
    QXmppMixInvitation(const QXmppMixInvitation &other);
    QXmppMixInvitation(QXmppMixInvitation &&);
    ~QXmppMixInvitation();

    QXmppMixInvitation &operator=(const QXmppMixInvitation &other);
    QXmppMixInvitation &operator=(QXmppMixInvitation &&);

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
