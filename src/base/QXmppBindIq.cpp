/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
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

#include <QDomElement>
#include <QTextStream>
#include <QXmlStreamWriter>

#include "QXmppBindIq.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

/// Returns the bound JID.
///

QString QXmppBindIq::jid() const
{
    return m_jid;
}

/// Sets the bound JID.
///
/// \param jid

void QXmppBindIq::setJid(const QString& jid)
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

void QXmppBindIq::setResource(const QString& resource)
{
    m_resource = resource;
}

/// \cond
bool QXmppBindIq::isBindIq(const QDomElement &element)
{
    QDomElement bindElement = element.firstChildElement("bind");
    return (bindElement.namespaceURI() == ns_bind);
}

void QXmppBindIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement bindElement = element.firstChildElement("bind");
    m_jid = bindElement.firstChildElement("jid").text();
    m_resource = bindElement.firstChildElement("resource").text();
}

void QXmppBindIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("bind");
    writer->writeAttribute("xmlns", ns_bind);
    if (!m_jid.isEmpty())
        helperToXmlAddTextElement(writer, "jid", m_jid);
    if (!m_resource.isEmpty())
        helperToXmlAddTextElement(writer, "resource", m_resource);
    writer->writeEndElement();
}
/// \endcond
