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

#include "QXmppConstants.h"
#include "QXmppMucIq.h"
#include "QXmppUtils.h"

QXmppDataForm QXmppMucOwnerIq::form() const
{
    return m_form;
}

void QXmppMucOwnerIq::setForm(const QXmppDataForm &form)
{
    m_form = form;
}

bool QXmppMucOwnerIq::isMucOwnerIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return (queryElement.namespaceURI() == ns_muc_owner);
}

void QXmppMucOwnerIq::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    setTypeFromStr(element.attribute("type"));

    QDomElement queryElement = element.firstChildElement("query");
    m_form.parse(queryElement.firstChildElement("x"));
}

void QXmppMucOwnerIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    helperToXmlAddAttribute(writer, "xmlns", ns_muc_owner);
    m_form.toXml(writer);
    writer->writeEndElement();
}

