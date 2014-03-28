/*
 * Copyright (C) 2008-2014 The QXmpp developers
 *
 * Author:
 *	Jeremy Lainé
 *
 * Source:
 *	https://github.com/qxmpp-project/qxmpp
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
#include "QXmppDialback.h"
#include "QXmppUtils.h"

/// Constructs a QXmppDialback.

QXmppDialback::QXmppDialback()
    : m_command(Result)
{
}

/// Returns the dialback command.

QXmppDialback::Command QXmppDialback::command() const
{
    return m_command;
}

/// Sets the dialback command.
///
/// \param command

void QXmppDialback::setCommand(QXmppDialback::Command command)
{
    m_command = command;
}

/// Returns the dialback key.

QString QXmppDialback::key() const
{
    return m_key;
}

/// Sets the dialback key.
///
/// \param key

void QXmppDialback::setKey(const QString &key)
{
    m_key = key;
}

/// Returns the dialback type.

QString QXmppDialback::type() const
{
    return m_type;
}

/// Sets the dialback type.
///
/// \param type

void QXmppDialback::setType(const QString &type)
{
    m_type = type;
}

/// \cond
bool QXmppDialback::isDialback(const QDomElement &element)
{
    return element.namespaceURI() == ns_server_dialback &&
           (element.tagName() == QLatin1String("result") ||
           element.tagName() == QLatin1String("verify"));
}

void QXmppDialback::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    if (element.tagName() == QLatin1String("result"))
        m_command = Result;
    else
        m_command = Verify;
    m_type = element.attribute("type");
    m_key = element.text();
}

void QXmppDialback::toXml(QXmlStreamWriter *xmlWriter) const
{
    if (m_command == Result)
        xmlWriter->writeStartElement("db:result");
    else
        xmlWriter->writeStartElement("db:verify");
    helperToXmlAddAttribute(xmlWriter, "id", id());
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", m_type);
    if (!m_key.isEmpty())
        xmlWriter->writeCharacters(m_key);
    xmlWriter->writeEndElement();
}
/// \endcond
