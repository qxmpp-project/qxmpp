/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Author:
 *	Manjeet Dahiya
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


#ifndef QXMPPROSTERIQ_H
#define QXMPPROSTERIQ_H

#include "QXmppIq.h"
#include <QList>
#include <QSet>

class QXmppRosterIq : public QXmppIq
{
public:

    class Item
    {
    public:
        enum SubscriptionType
        {
            NotSet = 0,
            None,
            Both,
            From,
            To,
            Remove
        };

        SubscriptionType subscriptionType() const;
        QString name() const;
        QString subscriptionStatus() const;
        QString bareJid() const;
        QSet<QString> groups() const;

        void setName(const QString&);
        void setSubscriptionStatus(const QString&);
        void addGroup(const QString&);
        void setBareJid(const QString&);
        void setSubscriptionType(SubscriptionType);
        QString getSubscriptionTypeStr() const;
        void setSubscriptionTypeFromStr(const QString&);
        void toXml(QXmlStreamWriter *writer) const;
        
// deprecated accessors, use the form without "get" instead
        SubscriptionType Q_DECL_DEPRECATED getSubscriptionType() const;
        QString Q_DECL_DEPRECATED getName() const;
        QString Q_DECL_DEPRECATED getSubscriptionStatus() const;
        QString Q_DECL_DEPRECATED getBareJid() const;
        QSet<QString> Q_DECL_DEPRECATED getGroups() const;

    private:
        QString m_bareJid;
        SubscriptionType m_type;
        QString m_name;
         // can be subscribe/unsubscribe (attribute "ask")
        QString m_subscriptionStatus;
        QSet<QString> m_groups;
    };

    QXmppRosterIq(QXmppIq::Type type);
    QXmppRosterIq(const QString& type);
    ~QXmppRosterIq();
    
    void addItem(const Item&);
    QList<Item> items() const;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;

// deprecated accessors, use the form without "get" instead
    QList<Item> Q_DECL_DEPRECATED getItems() const;

private:
    QList<Item> m_items;
};

#endif // QXMPPROSTERIQ_H
