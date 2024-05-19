// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppPingIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

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
    return isIqType(element, u"ping", ns_ping) && element.attribute(u"type"_s) == u"get";
}

/// \cond
void QXmppPingIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("ping"));
    writer->writeDefaultNamespace(toString65(ns_ping));
    writer->writeEndElement();
}
/// \endcond
