/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
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


#include "QXmppUtils.h"
#include "QXmppIq.h"

#include <QDomElement>
#include <QXmlStreamWriter>

static const char* iq_types[] = {
    "error",
    "get",
    "set",
    "result"
};

/// Constructs a QXmppIq with the specified \a type.
///
/// \param type

QXmppIq::QXmppIq(QXmppIq::Type type)
    : QXmppStanza(), m_type(type)
{
    generateAndSetNextId();
}

/// Returns the IQ's type.
///

QXmppIq::Type QXmppIq::type() const
{
    return m_type;
}

/// Sets the IQ's type.
///
/// \param type

void QXmppIq::setType(QXmppIq::Type type)
{
    m_type = type;
}

/// \cond
void QXmppIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    const QString type = element.attribute("type");
    for (int i = Error; i <= Result; i++) {
        if (type == iq_types[i]) {
            m_type = static_cast<Type>(i);
            break;
        }
    }

    parseElementFromChild(element);
}

void QXmppIq::parseElementFromChild(const QDomElement &element)
{
    QXmppElementList extensions;
    QDomElement itemElement = element.firstChildElement();
    while (!itemElement.isNull())
    {
        extensions.append(QXmppElement(itemElement));
        itemElement = itemElement.nextSiblingElement();
    }
    setExtensions(extensions);
}

void QXmppIq::toXml( QXmlStreamWriter *xmlWriter ) const
{
    xmlWriter->writeStartElement("iq");

    helperToXmlAddAttribute(xmlWriter, "id", id());
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", iq_types[m_type]);
    toXmlElementFromChild(xmlWriter);
    error().toXml(xmlWriter);
    xmlWriter->writeEndElement();
}

void QXmppIq::toXmlElementFromChild( QXmlStreamWriter *writer ) const
{
    foreach (const QXmppElement &extension, extensions())
        extension.toXml(writer);
}
/// \endcond
