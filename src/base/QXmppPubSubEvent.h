/*
 * Copyright (C) 2008-2020 The QXmpp developers
 *
 * Author:
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

#ifndef QXMPPPUBSUBEVENT_H
#define QXMPPPUBSUBEVENT_H

#include <QSharedData>

#include "QXmppElement.h"
#include "QXmppPubSubItem.h"

class QXmppPubSubEventPrivate;

///
/// \brief The QXmppPubSubEvent class represents a publish-subscribe event
/// as defined by XEP-0060: Publish-Subscribe.
///
class QXMPP_EXPORT QXmppPubSubEvent
{
public:
    QXmppPubSubEvent();
    QXmppPubSubEvent(const QXmppPubSubEvent &other);
    ~QXmppPubSubEvent();

    QXmppPubSubEvent& operator=(const QXmppPubSubEvent &other);

    QString nodeName() const;
    void setNodeName(const QString &nodeName);

    QList<QXmppPubSubItem> items() const;
    void setItems(const QList<QXmppPubSubItem> &items);

    bool isNull() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppPubSubEventPrivate> d;
};

#endif // QXMPPPUBSUBEVENT_H
