// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStartTlsPacket.h"

#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QXmlStreamWriter>

const static QStringList STARTTLS_TYPES = {
    QStringLiteral("starttls"),
    QStringLiteral("proceed"),
    QStringLiteral("failure")
};

/// Constructs a new QXmppStartTlsPacket
///
/// \param type The type of the new QXmppStartTlsPacket.

QXmppStartTlsPacket::QXmppStartTlsPacket(Type type)
    : m_type(type)
{
}

QXmppStartTlsPacket::~QXmppStartTlsPacket() = default;

/// Returns the type of the STARTTLS packet

QXmppStartTlsPacket::Type QXmppStartTlsPacket::type() const
{
    return m_type;
}

/// Sets the type of the STARTTLS packet

void QXmppStartTlsPacket::setType(QXmppStartTlsPacket::Type type)
{
    m_type = type;
}

/// \cond
void QXmppStartTlsPacket::parse(const QDomElement &element)
{
    if (!QXmppStartTlsPacket::isStartTlsPacket(element)) {
        return;
    }

    if (auto index = STARTTLS_TYPES.indexOf(element.tagName()); index >= 0) {
        m_type = Type(index);
    } else {
        m_type = Invalid;
    }
}

void QXmppStartTlsPacket::toXml(QXmlStreamWriter *writer) const
{
    if (m_type != Invalid) {
        writer->writeStartElement(STARTTLS_TYPES.at(int(m_type)));
        writer->writeDefaultNamespace(ns_tls);
        writer->writeEndElement();
    }
}
/// \endcond

/// Checks whether the given \p element is a STARTTLS packet according to
/// <a href="https://xmpp.org/rfcs/rfc6120.html#tls-process-initiate">RFC6120</a>.
///
/// \param element The element that should be checked for being a STARTTLS packet.
///
/// \returns True, if the element is a STARTTLS packet.

bool QXmppStartTlsPacket::isStartTlsPacket(const QDomElement &element)
{
    return element.namespaceURI() == ns_tls && STARTTLS_TYPES.contains(element.tagName());
}

/// Checks whether the given \p element is a STARTTLS packet according to
/// <a href="https://xmpp.org/rfcs/rfc6120.html#tls-process-initiate">RFC6120</a>
/// and has the correct type.
///
/// \param element The element that should be checked for being a STARTTLS packet.
/// \param type The type the element needs to have.
///
/// \returns True, if the element is a STARTTLS packet and has the correct type.

bool QXmppStartTlsPacket::isStartTlsPacket(const QDomElement &element, Type type)
{
    return element.namespaceURI() == ns_tls && element.tagName() == STARTTLS_TYPES.at(int(type));
}
