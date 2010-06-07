/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

/// \brief The QXmppMucAdminIq class represents a chat room administration IQ
/// as defined by XEP-0045: Multi-User Chat.
///
/// It is used to get or modify room memberships.
///

class QXmppMucAdminIq : public QXmppIq
{
public:
    class Item
    {
    public:
        QString affiliation() const;
        void setAffiliation(const QString &affiliation);

        QString jid() const;
        void setJid(const QString &jid);

        QString nick() const;
        void setNick(const QString &nick);

        QString reason() const;
        void setReason(const QString &reason);

        QString role() const;
        void setRole(const QString &role);

        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

    private:
        QString m_affiliation;
        QString m_jid;
        QString m_nick;
        QString m_reason;
        QString m_role;
    };

    QList<QXmppMucAdminIq::Item> items() const;
    void setItems(const QList<QXmppMucAdminIq::Item> &items);

    static bool isMucAdminIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QList<QXmppMucAdminIq::Item> m_items;
};

/// \brief The QXmppMucOwnerIq class represents a chat room configuration IQ as
/// defined by XEP-0045: Multi-User Chat.
///
/// It is used to get or modify room configuration options.
///
/// \sa QXmppDataForm
///

class QXmppMucOwnerIq : public QXmppIq
{
public:
    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);

    static bool isMucOwnerIq(const QDomElement &element);
    void parse(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

private:
    QXmppDataForm m_form;
};

#endif
