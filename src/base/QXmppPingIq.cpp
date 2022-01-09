/*
 * Copyright (C) 2008-2022 The QXmpp developers
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

#include "QXmppPingIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>

///
/// \class QXmppPingIq
///
/// QXmppPingIq represents a Ping IQ as defined by \xep{0199, XMPP Ping}.
///
/// \ingroup Stanzas
///

QXmppPingIq::QXmppPingIq()
    : QXmppIq(QXmppIq::Get)
{
}

///
/// Returns true, if the element is a ping IQ.
///
bool QXmppPingIq::isPingIq(const QDomElement &element)
{
    QDomElement pingElement = element.firstChildElement(QStringLiteral("ping"));
    return (element.attribute(QStringLiteral("type")) == QStringLiteral("get") &&
            pingElement.namespaceURI() == ns_ping);
}

/// \cond
void QXmppPingIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("ping"));
    writer->writeDefaultNamespace(ns_ping);
    writer->writeEndElement();
}
/// \endcond
