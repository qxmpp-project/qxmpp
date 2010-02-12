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

#include "QXmppElement.h"
#include "QXmppUtils.h"

#include <QDomElement>

QXmppElement::QXmppElement()
{
}

QXmppElement::QXmppElement(const QDomElement &element)
{
    m_tagName = element.tagName();
    QDomNamedNodeMap attributes = element.attributes();
    for (int i = 0; i < attributes.size(); i++)
    {
        QDomAttr attr = attributes.item(i).toAttr();
        m_attributes.insert(attr.name(), attr.value());
    }
}

QString QXmppElement::attribute(const QString &name) const
{
    return m_attributes.value(name);
}

void QXmppElement::setAttribute(const QString &name, const QString &value)
{
    m_attributes.insert(name, value);
}

QString QXmppElement::tagName() const
{
    return m_tagName;
}

void QXmppElement::setTagName(const QString &tagName)
{
    m_tagName = tagName;
}

void QXmppElement::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(m_tagName);
    foreach (const QString &attr, m_attributes.keys())
        helperToXmlAddAttribute(writer, attr, m_attributes.value(attr));
    writer->writeEndElement();
}

