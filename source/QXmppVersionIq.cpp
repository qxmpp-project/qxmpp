/*
 * Copyright (C) 2010 Bolloré telecom
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	http://code.google.com/p/qxmpp
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

#include "QXmppUtils.h"
#include "QXmppVersionIq.h"

static const char *ns_version = "jabber:iq:version";

QString QXmppVersionIq::name() const
{
    return m_name;
}

void QXmppVersionIq::setName(const QString &name)
{
    m_name = name;
}

QString QXmppVersionIq::os() const
{
    return m_os;
}

void QXmppVersionIq::setOs(const QString &os)
{
    m_os = os;
}

QString QXmppVersionIq::version() const
{
    return m_version;
}

void QXmppVersionIq::setVersion(const QString &version)
{
    m_version = version;
}

bool QXmppVersionIq::isVersionIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return queryElement.namespaceURI() == ns_version;
}

void QXmppVersionIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    setTypeFromStr(element.attribute("type"));

    QDomElement queryElement = element.firstChildElement("query");
    m_name = element.firstChildElement("name").text();
    m_os = element.firstChildElement("os").text();
    m_version = element.firstChildElement("version").text();
}

void QXmppVersionIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_version);

    if (!m_name.isEmpty())
        helperToXmlAddTextElement(writer, "name", m_name);

    if (!m_os.isEmpty())
        helperToXmlAddTextElement(writer, "os", m_os);

    if (!m_version.isEmpty())
        helperToXmlAddTextElement(writer, "version", m_version);

    writer->writeEndElement();
}

