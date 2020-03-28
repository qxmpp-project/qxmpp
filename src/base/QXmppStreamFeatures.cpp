/*
 * Copyright (C) 2008-2020 The QXmpp developers
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

#include "QXmppStreamFeatures.h"

#include "QXmppConstants_p.h"

#include <QDomElement>

class QXmppStreamFeaturesPrivate : public QSharedData
{
public:
    QXmppStreamFeaturesPrivate();

    QXmppStreamFeatures::Mode bindMode;
    QXmppStreamFeatures::Mode sessionMode;
    QXmppStreamFeatures::Mode nonSaslAuthMode;
    QXmppStreamFeatures::Mode tlsMode;
    QXmppStreamFeatures::Mode streamManagementMode;
    QXmppStreamFeatures::Mode csiMode;
    QXmppStreamFeatures::Mode registerMode;
    QStringList authMechanisms;
    QStringList compressionMethods;
};

QXmppStreamFeaturesPrivate::QXmppStreamFeaturesPrivate()
    : bindMode(QXmppStreamFeatures::Disabled),
      sessionMode(QXmppStreamFeatures::Disabled),
      nonSaslAuthMode(QXmppStreamFeatures::Disabled),
      tlsMode(QXmppStreamFeatures::Disabled),
      streamManagementMode(QXmppStreamFeatures::Disabled),
      csiMode(QXmppStreamFeatures::Disabled),
      registerMode(QXmppStreamFeatures::Disabled)
{
}

QXmppStreamFeatures::QXmppStreamFeatures()
    : d(new QXmppStreamFeaturesPrivate)
{
}

QXmppStreamFeatures::QXmppStreamFeatures(const QXmppStreamFeatures &) = default;

QXmppStreamFeatures::~QXmppStreamFeatures() = default;

QXmppStreamFeatures &QXmppStreamFeatures::operator=(const QXmppStreamFeatures &) = default;

QXmppStreamFeatures::Mode QXmppStreamFeatures::bindMode() const
{
    return d->bindMode;
}

void QXmppStreamFeatures::setBindMode(QXmppStreamFeatures::Mode mode)
{
    d->bindMode = mode;
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::sessionMode() const
{
    return d->sessionMode;
}

void QXmppStreamFeatures::setSessionMode(Mode mode)
{
    d->sessionMode = mode;
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::nonSaslAuthMode() const
{
    return d->nonSaslAuthMode;
}

void QXmppStreamFeatures::setNonSaslAuthMode(QXmppStreamFeatures::Mode mode)
{
    d->nonSaslAuthMode = mode;
}

QStringList QXmppStreamFeatures::authMechanisms() const
{
    return d->authMechanisms;
}

void QXmppStreamFeatures::setAuthMechanisms(const QStringList &mechanisms)
{
    d->authMechanisms = mechanisms;
}

QStringList QXmppStreamFeatures::compressionMethods() const
{
    return d->compressionMethods;
}

void QXmppStreamFeatures::setCompressionMethods(const QStringList &methods)
{
    d->compressionMethods = methods;
}

QXmppStreamFeatures::Mode QXmppStreamFeatures::tlsMode() const
{
    return d->tlsMode;
}

void QXmppStreamFeatures::setTlsMode(QXmppStreamFeatures::Mode mode)
{
    d->tlsMode = mode;
}

///
/// Returns the mode (disabled, enabled or required) for \xep{0198}: Stream
/// Management
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::streamManagementMode() const
{
    return d->streamManagementMode;
}

///
/// Sets the mode for \xep{0198}: Stream Management
///
/// \param mode The mode to set.
///
void QXmppStreamFeatures::setStreamManagementMode(QXmppStreamFeatures::Mode mode)
{
    d->streamManagementMode = mode;
}

///
/// Returns the mode for \xep{0352}: Client State Indication
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::clientStateIndicationMode() const
{
    return d->csiMode;
}

///
/// Sets the mode for \xep{0352}: Client State Indication
///
/// \param mode The mode to set.
///
void QXmppStreamFeatures::setClientStateIndicationMode(QXmppStreamFeatures::Mode mode)
{
    d->csiMode = mode;
}

///
/// Returns the mode for \xep{0077}: In-Band Registration
///
/// \since QXmpp 1.1
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::registerMode() const
{
    return d->registerMode;
}

///
/// Sets the mode for \xep{0077}: In-Band Registration
///
/// \param mode The mode to set.
///
/// \since QXmpp 1.1
///
void QXmppStreamFeatures::setRegisterMode(const QXmppStreamFeatures::Mode &registerMode)
{
    d->registerMode = registerMode;
}

/// \cond
bool QXmppStreamFeatures::isStreamFeatures(const QDomElement &element)
{
    return element.namespaceURI() == ns_stream &&
        element.tagName() == QStringLiteral("features");
}

static QXmppStreamFeatures::Mode readFeature(const QDomElement &element, const char *tagName, const char *tagNs)
{
    QDomElement subElement = element.firstChildElement(tagName);
    QXmppStreamFeatures::Mode mode = QXmppStreamFeatures::Disabled;
    while (!subElement.isNull()) {
        if (subElement.namespaceURI() == tagNs) {
            if (!subElement.firstChildElement(QStringLiteral("required")).isNull())
                mode = QXmppStreamFeatures::Required;
            else if (mode != QXmppStreamFeatures::Required)
                mode = QXmppStreamFeatures::Enabled;
        }
        subElement = subElement.nextSiblingElement(tagName);
    }
    return mode;
}

void QXmppStreamFeatures::parse(const QDomElement &element)
{
    d->bindMode = readFeature(element, "bind", ns_bind);
    d->sessionMode = readFeature(element, "session", ns_session);
    d->nonSaslAuthMode = readFeature(element, "auth", ns_authFeature);
    d->tlsMode = readFeature(element, "starttls", ns_tls);
    d->streamManagementMode = readFeature(element, "sm", ns_stream_management);
    d->csiMode = readFeature(element, "csi", ns_csi);
    d->registerMode = readFeature(element, "register", ns_register_feature);

    // parse advertised compression methods
    QDomElement compression = element.firstChildElement(QStringLiteral("compression"));
    if (compression.namespaceURI() == ns_compressFeature) {
        QDomElement subElement = compression.firstChildElement(QStringLiteral("method"));
        while (!subElement.isNull()) {
            d->compressionMethods << subElement.text();
            subElement = subElement.nextSiblingElement(QStringLiteral("method"));
        }
    }

    // parse advertised SASL Authentication mechanisms
    QDomElement mechs = element.firstChildElement(QStringLiteral("mechanisms"));
    if (mechs.namespaceURI() == ns_sasl) {
        QDomElement subElement = mechs.firstChildElement(QStringLiteral("mechanism"));
        while (!subElement.isNull()) {
            d->authMechanisms << subElement.text();
            subElement = subElement.nextSiblingElement(QStringLiteral("mechanism"));
        }
    }
}

static void writeFeature(QXmlStreamWriter *writer, const char *tagName, const char *tagNs, QXmppStreamFeatures::Mode mode)
{
    if (mode != QXmppStreamFeatures::Disabled) {
        writer->writeStartElement(tagName);
        writer->writeDefaultNamespace(tagNs);
        if (mode == QXmppStreamFeatures::Required)
            writer->writeEmptyElement(QStringLiteral("required"));
        writer->writeEndElement();
    }
}

void QXmppStreamFeatures::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("stream:features"));
    writeFeature(writer, "bind", ns_bind, d->bindMode);
    writeFeature(writer, "session", ns_session, d->sessionMode);
    writeFeature(writer, "auth", ns_authFeature, d->nonSaslAuthMode);
    writeFeature(writer, "starttls", ns_tls, d->tlsMode);
    writeFeature(writer, "sm", ns_stream_management, d->streamManagementMode);
    writeFeature(writer, "csi", ns_csi, d->csiMode);
    writeFeature(writer, "register", ns_register_feature, d->registerMode);

    if (!d->compressionMethods.isEmpty()) {
        writer->writeStartElement(QStringLiteral("compression"));
        writer->writeDefaultNamespace(ns_compressFeature);
        for (const auto &method : d->compressionMethods)
            writer->writeTextElement(QStringLiteral("method"), method);
        writer->writeEndElement();
    }
    if (!d->authMechanisms.isEmpty()) {
        writer->writeStartElement(QStringLiteral("mechanisms"));
        writer->writeDefaultNamespace(ns_sasl);
        for (const auto &mechanism : d->authMechanisms)
            writer->writeTextElement(QStringLiteral("mechanism"), mechanism);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
