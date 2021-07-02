/*
 * Copyright (C) 2008-2021 The QXmpp developers
 *
 * Author:
 *  Germán Márquez Mejía
 *  Melvin Keskin
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

#ifndef QXMPPOMEMOELEMENT_H
#define QXMPPOMEMOELEMENT_H

#include <optional>

#include "QXmppOmemoEnvelope.h"
#include "QXmppElement.h"

class QXmppOmemoElementPrivate;

class QXMPP_EXPORT QXmppOmemoElement
{
public:
    QXmppOmemoElement();
    QXmppOmemoElement(const QXmppOmemoElement &other);
    ~QXmppOmemoElement();

    QXmppOmemoElement& operator=(const QXmppOmemoElement &other);

    int senderDeviceId() const;
    void setSenderDeviceId(const int id);

    QByteArray payload() const;
    void setPayload(const QByteArray &payload);

    std::optional<QXmppOmemoEnvelope> searchEnvelope(const QString &recipientJid, int recipientDeviceId) const;
    void addEnvelope(const QString &recipientJid, QXmppOmemoEnvelope &envelope);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isOmemoElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppOmemoElementPrivate> d;
};

Q_DECLARE_TYPEINFO(QXmppOmemoElement, Q_MOVABLE_TYPE);

#endif // QXMPPOMEMOELEMENT_H
