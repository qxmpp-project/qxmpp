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

#ifndef QXMPPPUBSUBAFFILIATION_H
#define QXMPPPUBSUBAFFILIATION_H

#include <QSharedDataPointer>
#include <QString>

class QXmppPubSubAffiliationPrivate;
class QDomElement;
class QXmlStreamWriter;

class QXmppPubSubAffiliation
{
public:
    enum Affiliation {
        None,
        Member,
        Outcast,
        Owner,
        Publisher,
        PublishOnly,
    };

    QXmppPubSubAffiliation(Affiliation = None,
                           const QString &node = {},
                           const QString &jid = {});
    QXmppPubSubAffiliation(const QXmppPubSubAffiliation &);
    ~QXmppPubSubAffiliation();

    QXmppPubSubAffiliation &operator=(const QXmppPubSubAffiliation &);

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

#endif // QXMPPPUBSUBAFFILIATION_H
