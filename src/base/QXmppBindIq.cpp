// SPDX-FileCopyrightText: 2011 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppBindIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include <QDomElement>
#include <QTextStream>
#include <QXmlStreamWriter>

using namespace QXmpp::Private;

/// Returns the bound JID.
///

QString QXmppBindIq::jid() const
{
    return m_jid;
}

/// Sets the bound JID.
///
/// \param jid

void QXmppBindIq::setJid(const QString &jid)
{
    m_jid = jid;
}

/// Returns the requested resource.
///

QString QXmppBindIq::resource() const
{
    return m_resource;
}

/// Sets the requested resource.
///
/// \param resource

void QXmppBindIq::setResource(const QString &resource)
{
    m_resource = resource;
}

/// \cond
bool QXmppBindIq::isBindIq(const QDomElement &element)
{
    return isIqType(element, u"bind", ns_bind);
}

void QXmppBindIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement bindElement = firstChildElement(element, u"bind");
    m_jid = firstChildElement(bindElement, u"jid").text();
    m_resource = firstChildElement(bindElement, u"resource").text();
}

void QXmppBindIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("bind"));
    writer->writeDefaultNamespace(ns_bind);
    if (!m_jid.isEmpty()) {
        writeXmlTextElement(writer, u"jid", m_jid);
    }
    if (!m_resource.isEmpty()) {
        writeXmlTextElement(writer, u"resource", m_resource);
    }
    writer->writeEndElement();
}
/// \endcond
