/*
 * Copyright (C) 2008-2010 The QXmpp developers
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

#include <QDebug>
#include <QDomElement>

#include "QXmppConstants.h"
#include "QXmppStreamFeatures.h"

QXmppStreamFeatures::QXmppStreamFeatures()
    : m_bindAvailable(false),
    m_sessionAvailable(false),
    m_securityMode(QXmppConfiguration::TLSEnabled)
{
}

bool QXmppStreamFeatures::isBindAvailable() const
{
    return m_bindAvailable;
}

void QXmppStreamFeatures::setBindAvailable(bool available)
{
    m_bindAvailable = available;
}

bool QXmppStreamFeatures::isSessionAvailable() const
{
    return m_sessionAvailable;
}

void QXmppStreamFeatures::setSessionAvailable(bool available)
{
    m_sessionAvailable = available;
}

QList<QXmppConfiguration::SASLAuthMechanism> QXmppStreamFeatures::authMechanisms() const
{
    return m_authMechanisms;
}

void QXmppStreamFeatures::setAuthMechanisms(QList<QXmppConfiguration::SASLAuthMechanism> &mechanisms)
{
    m_authMechanisms = mechanisms;
}

QXmppConfiguration::StreamSecurityMode QXmppStreamFeatures::securityMode() const
{
    return m_securityMode;
}

void QXmppStreamFeatures::setSecurityMode(QXmppConfiguration::StreamSecurityMode mode)
{
    m_securityMode = mode;
}

void QXmppStreamFeatures::parse(const QDomElement &element)
{
    m_bindAvailable = !element.firstChildElement("bind").isNull();
    m_sessionAvailable = !element.firstChildElement("session").isNull();

    // parse advertised SASL Authentication mechanisms
    QDomElement mechs = element.firstChildElement("mechanisms");
    QDomElement subElement = mechs.firstChildElement("mechanism");
    qDebug("SASL Authentication mechanisms:");
    while(!subElement.isNull())
    {
        qDebug() << subElement.text();
        if (subElement.text() == QLatin1String("PLAIN"))
            m_authMechanisms << QXmppConfiguration::SASLPlain;
        else if (subElement.text() == QLatin1String("DIGEST-MD5"))
            m_authMechanisms << QXmppConfiguration::SASLDigestMD5;
        else if (subElement.text() == QLatin1String("ANONYMOUS"))
            m_authMechanisms << QXmppConfiguration::SASLAnonymous;
        subElement = subElement.nextSiblingElement("mechanism");
    }

    // parse advertised TLS mode
    QDomElement tlsElement = element.firstChildElement("starttls");
    if (!tlsElement.isNull())
    {
        if (tlsElement.firstChildElement().tagName() == "required")
            m_securityMode = QXmppConfiguration::TLSRequired;
        else
            m_securityMode = QXmppConfiguration::TLSEnabled;
    } else {
        m_securityMode = QXmppConfiguration::TLSDisabled;
    }
}

void QXmppStreamFeatures::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("stream:features");
    if (m_bindAvailable)
    {
        writer->writeStartElement("bind");
        writer->writeAttribute("xmlns", ns_bind);
        writer->writeEndElement();
    }
    if (m_sessionAvailable)
    {
        writer->writeStartElement("session");
        writer->writeAttribute("xmlns", ns_session);
        writer->writeEndElement();
    }
    if (!m_authMechanisms.isEmpty())
    {
        writer->writeStartElement("mechanisms");
        writer->writeAttribute("xmlns", ns_sasl);
        for (int i = 0; i < m_authMechanisms.size(); i++)
        {
            writer->writeStartElement("mechanism");
            switch (m_authMechanisms[i])
            {
            case QXmppConfiguration::SASLPlain:
                writer->writeCharacters("PLAIN");
                break;
            case QXmppConfiguration::SASLDigestMD5:
                writer->writeCharacters("DIGEST-MD5");
                break;
            case QXmppConfiguration::SASLAnonymous:
                writer->writeCharacters("ANONYMOUS");
                break;
            }
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
    if (m_securityMode != QXmppConfiguration::TLSDisabled)
    {
        writer->writeStartElement("starttls");
        writer->writeAttribute("xmlns", ns_tls);
        if (m_securityMode == QXmppConfiguration::TLSRequired)
            writer->writeEmptyElement("required");
        writer->writeEndElement();
    }
    writer->writeEndElement();
}

