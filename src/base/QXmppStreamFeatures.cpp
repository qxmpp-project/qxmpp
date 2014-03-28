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
#include "QXmppStreamFeatures.h"

QXmppStreamFeatures::QXmppStreamFeatures()
    : m_bindMode(Disabled),
    m_sessionMode(Disabled),
    m_nonSaslAuthMode(Disabled),
    m_tlsMode(Disabled)
{
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::bindMode() const
{
    return m_bindMode;
}

void QXmppStreamFeatures::setBindMode(QXmppStreamFeatures::Mode mode)
{
    m_bindMode = mode;
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::sessionMode() const
{
    return m_sessionMode;
}

void QXmppStreamFeatures::setSessionMode(Mode mode)
{
    m_sessionMode = mode;
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::nonSaslAuthMode() const
{
    return m_nonSaslAuthMode;
}

void QXmppStreamFeatures::setNonSaslAuthMode(QXmppStreamFeatures::Mode mode)
{
    m_nonSaslAuthMode = mode;
}

QStringList QXmppStreamFeatures::authMechanisms() const
{
    return m_authMechanisms;
}

void QXmppStreamFeatures::setAuthMechanisms(const QStringList &mechanisms)
{
    m_authMechanisms = mechanisms;
}

QStringList QXmppStreamFeatures::compressionMethods() const
{
    return m_compressionMethods;
}

void QXmppStreamFeatures::setCompressionMethods(const QStringList &methods)
{
    m_compressionMethods = methods;
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::tlsMode() const
{
    return m_tlsMode;
}

void QXmppStreamFeatures::setTlsMode(QXmppStreamFeatures::Mode mode)
{
    m_tlsMode = mode;
}

/// \cond
bool QXmppStreamFeatures::isStreamFeatures(const QDomElement &element)
{
    return element.namespaceURI() == ns_stream &&
           element.tagName() == "features";
}

static QXmppStreamFeatures::Mode readFeature(const QDomElement &element, const char *tagName, const char *tagNs)
{
    QDomElement subElement = element.firstChildElement(tagName);
    if (subElement.namespaceURI() == tagNs)
    {
        if (!subElement.firstChildElement("required").isNull())
            return QXmppStreamFeatures::Required;
        else
            return QXmppStreamFeatures::Enabled;
    } else {
        return QXmppStreamFeatures::Disabled;
    }
}

void QXmppStreamFeatures::parse(const QDomElement &element)
{
    m_bindMode = readFeature(element, "bind", ns_bind);
    m_sessionMode = readFeature(element, "session", ns_session);
    m_nonSaslAuthMode = readFeature(element, "auth", ns_authFeature);
    m_tlsMode = readFeature(element, "starttls", ns_tls);

    // parse advertised compression methods
    QDomElement compression = element.firstChildElement("compression");
    if (compression.namespaceURI() == ns_compressFeature)
    {
        QDomElement subElement = compression.firstChildElement("method");
        while(!subElement.isNull())
        {
            m_compressionMethods << subElement.text();
            subElement = subElement.nextSiblingElement("method");
        }
    }

    // parse advertised SASL Authentication mechanisms
    QDomElement mechs = element.firstChildElement("mechanisms");
    if (mechs.namespaceURI() == ns_sasl)
    {
        QDomElement subElement = mechs.firstChildElement("mechanism");
        while(!subElement.isNull()) {
            m_authMechanisms << subElement.text();
            subElement = subElement.nextSiblingElement("mechanism");
        }
    }
}

static void writeFeature(QXmlStreamWriter *writer, const char *tagName, const char *tagNs, QXmppStreamFeatures::Mode mode)
{
    if (mode != QXmppStreamFeatures::Disabled)
    {
        writer->writeStartElement(tagName);
        writer->writeAttribute("xmlns", tagNs);
        if (mode == QXmppStreamFeatures::Required)
            writer->writeEmptyElement("required");
        writer->writeEndElement();
    }
}

void QXmppStreamFeatures::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("stream:features");
    writeFeature(writer, "bind", ns_bind, m_bindMode);
    writeFeature(writer, "session", ns_session, m_sessionMode);
    writeFeature(writer, "auth", ns_authFeature, m_nonSaslAuthMode);
    writeFeature(writer, "starttls", ns_tls, m_tlsMode);

    if (!m_compressionMethods.isEmpty())
    {
        writer->writeStartElement("compression");
        writer->writeAttribute("xmlns", ns_compressFeature);
        foreach (const QString &method, m_compressionMethods)
            writer->writeTextElement("method", method);
        writer->writeEndElement();
    }
    if (!m_authMechanisms.isEmpty())
    {
        writer->writeStartElement("mechanisms");
        writer->writeAttribute("xmlns", ns_sasl);
        foreach (const QString &mechanism, m_authMechanisms)
            writer->writeTextElement("mechanism",  mechanism);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
