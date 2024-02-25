// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppSessionIq.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

bool QXmppSessionIq::isSessionIq(const QDomElement &element)
{
    return isIqType(element, u"session", ns_session);
}

void QXmppSessionIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("session"));
    writer->writeDefaultNamespace(toString65(ns_session));
    writer->writeEndElement();
}