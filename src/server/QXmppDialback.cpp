// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppDialback.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

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
void QXmppDialback::setType(const QString &type)
{
    m_type = type;
}

/// \cond
bool QXmppDialback::isDialback(const QDomElement &element)
{
    return element.namespaceURI() == ns_server_dialback &&
        (element.tagName() == u"result" ||
         element.tagName() == u"verify");
}

void QXmppDialback::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);
    if (element.tagName() == u"result") {
        m_command = Result;
    } else {
        m_command = Verify;
    }
    m_type = element.attribute(u"type"_s);
    m_key = element.text();
}

void QXmppDialback::toXml(QXmlStreamWriter *xmlWriter) const
{
    if (m_command == Result) {
        xmlWriter->writeStartElement(QSL65("db:result"));
    } else {
        xmlWriter->writeStartElement(QSL65("db:verify"));
    }
    writeOptionalXmlAttribute(xmlWriter, u"id", id());
    writeOptionalXmlAttribute(xmlWriter, u"to", to());
    writeOptionalXmlAttribute(xmlWriter, u"from", from());
    writeOptionalXmlAttribute(xmlWriter, u"type", m_type);
    if (!m_key.isEmpty()) {
        xmlWriter->writeCharacters(m_key);
    }
    xmlWriter->writeEndElement();
}
/// \endcond
