// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
