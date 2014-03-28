/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
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

#include "QXmppConstants.h"
#include "QXmppUtils.h"
#include "QXmppVersionIq.h"

/// Returns the name of the software.
///

QString QXmppVersionIq::name() const
{
    return m_name;
}

/// Sets the name of the software.
///
/// \param name

void QXmppVersionIq::setName(const QString &name)
{
    m_name = name;
}

/// Returns the operating system.
///

QString QXmppVersionIq::os() const
{
    return m_os;
}

/// Sets the operating system.
///
/// \param os

void QXmppVersionIq::setOs(const QString &os)
{
    m_os = os;
}

/// Returns the software version.
///

QString QXmppVersionIq::version() const
{
    return m_version;
}

/// Sets the software version.
///
/// \param version

void QXmppVersionIq::setVersion(const QString &version)
{
    m_version = version;
}

/// \cond
bool QXmppVersionIq::isVersionIq(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    return queryElement.namespaceURI() == ns_version;
}

void QXmppVersionIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement queryElement = element.firstChildElement("query");
    m_name = queryElement.firstChildElement("name").text();
    m_os = queryElement.firstChildElement("os").text();
    m_version = queryElement.firstChildElement("version").text();
}

void QXmppVersionIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("query");
    writer->writeAttribute("xmlns", ns_version);

    if (!m_name.isEmpty())
        helperToXmlAddTextElement(writer, "name", m_name);

    if (!m_os.isEmpty())
        helperToXmlAddTextElement(writer, "os", m_os);

    if (!m_version.isEmpty())
        helperToXmlAddTextElement(writer, "version", m_version);

    writer->writeEndElement();
}
/// \endcond
