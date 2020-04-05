/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#ifndef QXMPPROSTERIQ_H
#define QXMPPROSTERIQ_H

#include "QXmppIq.h"

#include <QList>
#include <QSet>
#include <QSharedDataPointer>

class QXmppRosterIqPrivate;

/// \brief The QXmppRosterIq class represents a roster IQ.
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppRosterIq : public QXmppIq
{
public:
    class ItemPrivate;

    /// \brief The QXmppRosterIq::Item class represents a roster entry.
    class QXMPP_EXPORT Item
    {
    public:
        /// An enumeration for type of subscription with the bareJid in the roster.
        enum SubscriptionType {
            None = 0,    ///< the user does not have a subscription to the
                         ///< contact's presence information, and the contact does
                         ///< not have a subscription to the user's presence information
            From = 1,    ///< the contact has a subscription to the user's presence information,
                         ///< but the user does not have a subscription to the contact's presence information
            To = 2,      ///< the user has a subscription to the contact's presence information,
                         ///< but the contact does not have a subscription to the user's presence information
            Both = 3,    ///< both the user and the contact have subscriptions to each
                         ///< other's presence information
            Remove = 4,  ///< to delete a roster item
            NotSet = 8   ///< the subscription state was not specified
        };

        Item();
        Item(const Item &other);
        ~Item();

        Item &operator=(const Item &other);

        QString bareJid() const;
        QSet<QString> groups() const;
        QString name() const;
        QString subscriptionStatus() const;
        SubscriptionType subscriptionType() const;
        bool isApproved() const;

        void setBareJid(const QString &);
        void setGroups(const QSet<QString> &);
        void setName(const QString &);
        void setSubscriptionStatus(const QString &);
        void setSubscriptionType(SubscriptionType);
        void setIsApproved(bool);

        // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
        bool isMixChannel() const;
        void setIsMixChannel(bool);

        QString mixParticipantId() const;
        void setMixParticipantId(const QString &);

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
        /// \endcond

    private:
        QString getSubscriptionTypeStr() const;
        void setSubscriptionTypeFromStr(const QString &);

        QSharedDataPointer<ItemPrivate> d;
    };

    QXmppRosterIq();
    QXmppRosterIq(const QXmppRosterIq &);
    ~QXmppRosterIq() override;

    QXmppRosterIq &operator=(const QXmppRosterIq &);

    QString version() const;
    void setVersion(const QString &);

    void addItem(const Item &);
    QList<Item> items() const;

    // XEP-0405: Mediated Information eXchange (MIX): Participant Server Requirements
    bool mixAnnotate() const;
    void setMixAnnotate(bool);

    /// \cond
    static bool isRosterIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppRosterIqPrivate> d;
};

#endif  // QXMPPROSTERIQ_H
