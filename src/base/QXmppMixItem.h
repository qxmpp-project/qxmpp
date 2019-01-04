/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Linus Jahn <lnj@kaidan.im>
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

#ifndef QXMPPMIXITEM_H
#define QXMPPMIXITEM_H

#include "QXmppElement.h"

class QXmppMixInfoItemPrivate;
class QXmppMixParticipantItemPrivate;

/// \brief The QXmppMixInfoItem class represents a PubSub item of a MIX
/// channel containing channel information as defined by XEP-0369: Mediated
/// Information eXchange (MIX).
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMixInfoItem
{
public:
    QXmppMixInfoItem();

    QString name() const;
    void setName(const QString&);

    QString description() const;
    void setDescription(const QString&);

    QStringList contactJids() const;
    void setContactJids(const QStringList&);

    void parse(const QXmppElement& itemContent);
    QXmppElement toElement() const;

    static bool isMixChannelInfo(const QDomElement&);

private:
    QXmppMixInfoItemPrivate *d;
};

/// \brief The QXmppMixParticipantItem class represents a PubSub item of a MIX
/// channel participant as defined by XEP-0369: Mediated Information eXchange
/// (MIX).
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMixParticipantItem
{
public:
    QXmppMixParticipantItem();

    QString nick() const;
    void setNick(const QString&);

    QString jid() const;
    void setJid(const QString&);

    void parse(const QXmppElement& itemContent);
    QXmppElement toElement() const;

    static bool isMixParticipantItem(const QDomElement&);

private:
    QXmppMixParticipantItemPrivate *d;
};

#endif // QXMPPMIXITEM_H
