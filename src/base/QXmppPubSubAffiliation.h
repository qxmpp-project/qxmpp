// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBAFFILIATION_H
#define QXMPPPUBSUBAFFILIATION_H

#include "QXmppGlobal.h"

#include <QMetaType>
#include <QSharedDataPointer>

class QXmppPubSubAffiliationPrivate;
class QDomElement;
class QXmlStreamWriter;

class QXMPP_EXPORT QXmppPubSubAffiliation
{
public:
    ///
    /// This enum describes the type of the affiliation of the user with the
    /// node.
    ///
    enum Affiliation {
        None,         ///< No affiliation, but may subscribe.
        Member,       ///< Active member, is subscribed, can read.
        Outcast,      ///< Cannot subscribe, cannot read, 'banned'.
        Owner,        ///< Highest privileges, can read, publish & configure.
        Publisher,    ///< May read and publish, but cannot configure node.
        PublishOnly,  ///< Can only publish, cannot subscribe.
    };

    QXmppPubSubAffiliation(Affiliation = None,
                           const QString &node = {},
                           const QString &jid = {});
    QXmppPubSubAffiliation(const QXmppPubSubAffiliation &);
    QXmppPubSubAffiliation(QXmppPubSubAffiliation &&);
    ~QXmppPubSubAffiliation();

    QXmppPubSubAffiliation &operator=(const QXmppPubSubAffiliation &);
    QXmppPubSubAffiliation &operator=(QXmppPubSubAffiliation &&);

    Affiliation type() const;
    void setType(Affiliation type);

    QString node() const;
    void setNode(const QString &node);

    QString jid() const;
    void setJid(const QString &jid);

    static bool isAffiliation(const QDomElement &);

    /// \cond
    void parse(const QDomElement &);
    void toXml(QXmlStreamWriter *) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppPubSubAffiliationPrivate> d;
};

Q_DECLARE_METATYPE(QXmppPubSubAffiliation)
Q_DECLARE_METATYPE(QXmppPubSubAffiliation::Affiliation)

#endif  // QXMPPPUBSUBAFFILIATION_H
