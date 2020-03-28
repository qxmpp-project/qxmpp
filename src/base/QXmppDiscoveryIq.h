/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#ifndef QXMPPDISCOVERY_H
#define QXMPPDISCOVERY_H

#include "QXmppDataForm.h"
#include "QXmppIq.h"

#include <QSharedDataPointer>

class QXmppDiscoveryIdentityPrivate;
class QXmppDiscoveryItemPrivate;
class QXmppDiscoveryIqPrivate;

///
/// \brief QXmppDiscoveryIq represents a discovery IQ request or result
/// containing a list of features and other information about an entity as
/// defined by \xep{0030}: Service Discovery.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppDiscoveryIq : public QXmppIq
{
public:
    ///
    /// \brief Identity represents one of possibly multiple identities of an
    /// XMPP entity obtained from a service discovery request as defined in
    /// \xep{0030}: Service Discovery.
    ///
    class QXMPP_EXPORT Identity
    {
    public:
        Identity();
        Identity(const Identity &other);
        ~Identity();

        Identity &operator=(const Identity &other);

        QString category() const;
        void setCategory(const QString &category);

        QString language() const;
        void setLanguage(const QString &language);

        QString name() const;
        void setName(const QString &name);

        QString type() const;
        void setType(const QString &type);

    private:
        QSharedDataPointer<QXmppDiscoveryIdentityPrivate> d;
    };

    ///
    /// \brief Item represents a related XMPP entity that can be queried using
    /// \xep{0030}: Service Discovery.
    ///
    class QXMPP_EXPORT Item
    {
    public:
        Item();
        Item(const Item &);
        ~Item();

        Item &operator=(const Item &);

        QString jid() const;
        void setJid(const QString &jid);

        QString name() const;
        void setName(const QString &name);

        QString node() const;
        void setNode(const QString &node);

    private:
        QSharedDataPointer<QXmppDiscoveryItemPrivate> d;
    };

    QXmppDiscoveryIq();
    QXmppDiscoveryIq(const QXmppDiscoveryIq &);
    ~QXmppDiscoveryIq();

    QXmppDiscoveryIq &operator=(const QXmppDiscoveryIq &);

    enum QueryType {
        InfoQuery,
        ItemsQuery
    };

    QStringList features() const;
    void setFeatures(const QStringList &features);

    QList<QXmppDiscoveryIq::Identity> identities() const;
    void setIdentities(const QList<QXmppDiscoveryIq::Identity> &identities);

    QList<QXmppDiscoveryIq::Item> items() const;
    void setItems(const QList<QXmppDiscoveryIq::Item> &items);

    QXmppDataForm form() const;
    void setForm(const QXmppDataForm &form);

    QString queryNode() const;
    void setQueryNode(const QString &node);

    enum QueryType queryType() const;
    void setQueryType(enum QueryType type);

    QByteArray verificationString() const;

    static bool isDiscoveryIq(const QDomElement &element);

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppDiscoveryIqPrivate> d;
};

#endif
