/*
 * Copyright (C) 2008-2010 Manjeet Dahiya
 *
 * Authors:
 *	Manjeet Dahiya
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
#include <QTextStream>
#include <QXmlStreamWriter>

#include "QXmppBind.h"
#include "QXmppUtils.h"
#include "QXmppConstants.h"

QXmppBind::QXmppBind(QXmppIq::Type type)
    : QXmppIq(type)
{
}

QXmppBind::QXmppBind(const QString& type)
    : QXmppIq(type)
{
}

QString QXmppBind::jid() const
{
    return m_jid;
}

QString QXmppBind::resource() const
{
    return m_resource;
}

void QXmppBind::setJid(const QString& str)
{
    m_jid = str;
}

void QXmppBind::setResource(const QString& str)
{
    m_resource = str;
}

bool QXmppBind::isBind(const QDomElement &element)
{
    QDomElement bindElement = element.firstChildElement("bind");
    return (bindElement.namespaceURI() == ns_bind);
}

void QXmppBind::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    setTypeFromStr(element.attribute("type"));

    QDomElement bindElement = element.firstChildElement("bind");
    m_jid = bindElement.firstChildElement("jid").text();
    m_resource = bindElement.firstChildElement("resource").text();
}

void QXmppBind::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("bind");
    helperToXmlAddAttribute(writer, "xmlns", ns_bind);
    helperToXmlAddTextElement(writer, "jid", m_jid);
    helperToXmlAddTextElement(writer, "resource", m_resource);
    writer->writeEndElement();
}

QString QXmppBind::getJid() const
{
    return m_jid;
}

QString QXmppBind::getResource() const
{
    return m_resource;
}
