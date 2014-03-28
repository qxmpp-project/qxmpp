/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
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

#ifndef QXMPPMUCIQ_H
#define QXMPPMUCIQ_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"

/// \brief The QXmppMucItem class represents a chat room "item".
///
/// It is used to convey information such as permissions.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMucItem
{
public:
    /// This enum is used to represent long-lived permissions in a room (affiliations).
    enum Affiliation {
        UnspecifiedAffiliation,
        OutcastAffiliation,
        NoAffiliation,
        MemberAffiliation,
        AdminAffiliation,
        OwnerAffiliation,
    };

    /// This enum is used to represent short-lived permissions in a room (roles).
    enum Role {
        UnspecifiedRole,
        NoRole,
        VisitorRole,
        ParticipantRole,
        ModeratorRole,
    };

    QXmppMucItem();
    bool isNull() const;

    QString actor() const;
    void setActor(const QString &actor);

    Affiliation affiliation() const;
    void setAffiliation(Affiliation affiliation);

    QString jid() const;
    void setJid(const QString &jid);

    QString nick() const;
    void setNick(const QString &nick);

    QString reason() const;
    void setReason(const QString &reason);

    Role role() const;
    void setRole(Role role);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    static Affiliation affiliationFromString(const QString &affiliationStr);
    static QString affiliationToString(Affiliation affiliation);
    static Role roleFromString(const QString &roleStr);
    static QString roleToString(Role role);
    /// \endcond
private:
    QString m_actor;
    Affiliation m_affiliation;
    QString m_jid;
    QString m_nick;
    QString m_reason;
    Role m_role;
};

/// \brief The QXmppMucAdminIq class represents a chat room administration IQ
/// as defined by XEP-0045: Multi-User Chat.
///
/// It is used to get or modify room memberships.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppMucAdminIq : public QXmppIq
{
public:
    QList<QXmppMucItem> items() const;
    void setItems(const QList<QXmppMucItem> &items);

    /// \cond
    static bool isMucAdminIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QList<QXmppMucItem> m_items;
};

/// \brief The QXmppMucOwnerIq class represents a chat room configuration IQ as
/// defined by XEP-0045: Multi-User Chat.
///
/// It is used to get or modify room configuration options.
///
/// \sa QXmppDataForm
///

class QXMPP_EXPORT QXmppMucOwnerIq : public QXmppIq
{
public:
    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);

    /// \cond
    static bool isMucOwnerIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QXmppDataForm m_form;
};

#endif
