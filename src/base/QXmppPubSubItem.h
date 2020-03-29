/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
 *  Jeremy Lainé
 *  Germán Márquez Mejía
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

#ifndef QXMPPPUBSUBITEM_H
#define QXMPPPUBSUBITEM_H

#include "QXmppGlobal.h"

#include <QSharedDataPointer>

class QDomElement;
class QXmlStreamWriter;

class QXmppElement;
class QXmppPubSubItemPrivate;

///
/// \brief The QXmppPubSubItem class represents a publish-subscribe item
/// as defined by \xep{0060}: Publish-Subscribe.
///
class QXMPP_EXPORT QXmppPubSubItem
{
public:
    QXmppPubSubItem();
    QXmppPubSubItem(const QXmppPubSubItem &other);
    QXmppPubSubItem(const QString &id);
    QXmppPubSubItem(const QXmppElement &payload);
    QXmppPubSubItem(const QString &id, const QXmppElement &payload);
    ~QXmppPubSubItem();

    QXmppPubSubItem &operator=(const QXmppPubSubItem &other);

    QString id() const;
    void setId(const QString &id);

    QXmppElement payload() const;
    void setPayload(const QXmppElement &payload);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppPubSubItemPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppPubSubItem, Q_MOVABLE_TYPE);

#endif // QXMPPPUBSUBITEM_H
