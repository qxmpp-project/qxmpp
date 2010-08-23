/*
 * Copyright (C) 2008-2010 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
 *
 * Source:
 *  http://code.google.com/p/qxmpp
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

QXmppBindIq::QXmppBindIq()
{
}

QXmppBindIq::QXmppBindIq(QXmppIq::Type type)
    : QXmppIq(type)
{
}

QString QXmppBindIq::jid() const
{
    return m_jid;
}

QString QXmppBindIq::resource() const
{
    return m_resource;
}

void QXmppBindIq::setJid(const QString& str)
{
    m_jid = str;
}

void QXmppBindIq::setResource(const QString& str)
{
    m_resource = str;
}

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
    helperToXmlAddAttribute(writer, "xmlns", ns_bind);
    if (!m_jid.isEmpty())
        helperToXmlAddTextElement(writer, "jid", m_jid);
    if (!m_resource.isEmpty())
        helperToXmlAddTextElement(writer, "resource", m_resource);
    writer->writeEndElement();
}

