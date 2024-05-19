// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppStreamFeatures.h"

#include "QXmppConstants_p.h"
#include "QXmppGlobal_p.h"
#include "QXmppSasl_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

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
    bool preApprovedSubscriptionsSupported;
    bool rosterVersioningSupported;
    QStringList authMechanisms;
    QStringList compressionMethods;
    std::optional<Sasl2::StreamFeature> sasl2Feature;
};

QXmppStreamFeaturesPrivate::QXmppStreamFeaturesPrivate()
    : bindMode(QXmppStreamFeatures::Disabled),
      sessionMode(QXmppStreamFeatures::Disabled),
      nonSaslAuthMode(QXmppStreamFeatures::Disabled),
      tlsMode(QXmppStreamFeatures::Disabled),
      streamManagementMode(QXmppStreamFeatures::Disabled),
      csiMode(QXmppStreamFeatures::Disabled),
      registerMode(QXmppStreamFeatures::Disabled),
      preApprovedSubscriptionsSupported(false),
      rosterVersioningSupported(false)
{
}

QXmppStreamFeatures::QXmppStreamFeatures()
    : d(new QXmppStreamFeaturesPrivate)
{
}

/// Default copy-constructor
QXmppStreamFeatures::QXmppStreamFeatures(const QXmppStreamFeatures &) = default;
/// Default move-constructor
QXmppStreamFeatures::QXmppStreamFeatures(QXmppStreamFeatures &&) = default;
QXmppStreamFeatures::~QXmppStreamFeatures() = default;
/// Default assignment operator
QXmppStreamFeatures &QXmppStreamFeatures::operator=(const QXmppStreamFeatures &) = default;
/// Default move-assignment operator
QXmppStreamFeatures &QXmppStreamFeatures::operator=(QXmppStreamFeatures &&) = default;

///
/// Returns the resource binding mode
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::bindMode() const
{
    return d->bindMode;
}

///
/// Sets the resource binding mode
///
void QXmppStreamFeatures::setBindMode(QXmppStreamFeatures::Mode mode)
{
    d->bindMode = mode;
}

///
/// Returns session mode
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::sessionMode() const
{
    return d->sessionMode;
}

///
/// Sets session mode
///
void QXmppStreamFeatures::setSessionMode(Mode mode)
{
    d->sessionMode = mode;
}

///
/// Returns Non-SASL Authentication mode
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::nonSaslAuthMode() const
{
    return d->nonSaslAuthMode;
}

///
/// Sets Non-SASL Authentication mode
///
void QXmppStreamFeatures::setNonSaslAuthMode(QXmppStreamFeatures::Mode mode)
{
    d->nonSaslAuthMode = mode;
}

///
/// Returns the available authentication mechanisms
///
QStringList QXmppStreamFeatures::authMechanisms() const
{
    return d->authMechanisms;
}

///
/// Sets the available authentication mechanisms
///
void QXmppStreamFeatures::setAuthMechanisms(const QStringList &mechanisms)
{
    d->authMechanisms = mechanisms;
}

///
/// Returns the \xep{0388, Extensible SASL Profile} stream feature.
///
const std::optional<Sasl2::StreamFeature> &QXmppStreamFeatures::sasl2Feature() const
{
    return d->sasl2Feature;
}

///
/// Sets the \xep{0388, Extensible SASL Profile} stream feature.
///
void QXmppStreamFeatures::setSasl2Feature(const std::optional<Sasl2::StreamFeature> &feature)
{
    d->sasl2Feature = feature;
}

///
/// Returns the available compression methods
///
QStringList QXmppStreamFeatures::compressionMethods() const
{
    return d->compressionMethods;
}

///
/// Sets the available compression methods
///
void QXmppStreamFeatures::setCompressionMethods(const QStringList &methods)
{
    d->compressionMethods = methods;
}

///
/// Returns the mode for STARTTLS
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::tlsMode() const
{
    return d->tlsMode;
}

///
/// Sets the mode for STARTTLS
///
void QXmppStreamFeatures::setTlsMode(QXmppStreamFeatures::Mode mode)
{
    d->tlsMode = mode;
}

///
/// Returns the mode (disabled, enabled or required) for \xep{0198, Stream
/// Management}
///
/// \since QXmpp 1.0
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::streamManagementMode() const
{
    return d->streamManagementMode;
}

///
/// Sets the mode for \xep{0198, Stream Management}
///
/// \param mode The mode to set.
///
/// \since QXmpp 1.0
///
void QXmppStreamFeatures::setStreamManagementMode(QXmppStreamFeatures::Mode mode)
{
    d->streamManagementMode = mode;
}

///
/// Returns the mode for \xep{0352, Client State Indication}
///
/// \since QXmpp 1.0
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::clientStateIndicationMode() const
{
    return d->csiMode;
}

///
/// Sets the mode for \xep{0352, Client State Indication}
///
/// \param mode The mode to set.
///
/// \since QXmpp 1.0
///
void QXmppStreamFeatures::setClientStateIndicationMode(QXmppStreamFeatures::Mode mode)
{
    d->csiMode = mode;
}

///
/// Returns the mode for \xep{0077, In-Band Registration}
///
/// \since QXmpp 1.1
///
QXmppStreamFeatures::Mode QXmppStreamFeatures::registerMode() const
{
    return d->registerMode;
}

///
/// Sets the mode for \xep{0077, In-Band Registration}
///
/// \param mode The mode to set.
///
/// \since QXmpp 1.1
///
void QXmppStreamFeatures::setRegisterMode(const QXmppStreamFeatures::Mode &mode)
{
    d->registerMode = mode;
}

///
/// Returns whether usage of Pre-Approved roster subscriptions is supported.
///
/// \since QXmpp 1.3
///
bool QXmppStreamFeatures::preApprovedSubscriptionsSupported() const
{
    return d->preApprovedSubscriptionsSupported;
}

///
/// Sets whether usage of Pre-Approved roster subscriptions is supported.
///
/// \since QXmpp 1.3
///
void QXmppStreamFeatures::setPreApprovedSubscriptionsSupported(bool supported)
{
    d->preApprovedSubscriptionsSupported = supported;
}

///
/// Returns whether roster versioning from RFC6121 is supported.
///
/// \since QXmpp 1.3
///
bool QXmppStreamFeatures::rosterVersioningSupported() const
{
    return d->rosterVersioningSupported;
}

///
/// Sets whether roster versioning from RFC6121 is supported.
///
/// \since QXmpp 1.3
///
void QXmppStreamFeatures::setRosterVersioningSupported(bool supported)
{
    d->rosterVersioningSupported = supported;
}

/// \cond
bool QXmppStreamFeatures::isStreamFeatures(const QDomElement &element)
{
    return element.namespaceURI() == ns_stream &&
        element.tagName() == u"features";
}

static QXmppStreamFeatures::Mode readFeature(const QDomElement &element, QStringView tagName, QStringView tagNs)
{
    if (auto subEl = firstChildElement(element, tagName, tagNs); !subEl.isNull()) {
        if (!firstChildElement(subEl, u"required").isNull()) {
            return QXmppStreamFeatures::Required;
        }
        return QXmppStreamFeatures::Enabled;
    }
    return QXmppStreamFeatures::Disabled;
}

void QXmppStreamFeatures::parse(const QDomElement &element)
{
    d->bindMode = readFeature(element, u"bind", ns_bind);
    d->sessionMode = readFeature(element, u"session", ns_session);
    d->nonSaslAuthMode = readFeature(element, u"auth", ns_authFeature);
    d->tlsMode = readFeature(element, u"starttls", ns_tls);
    d->streamManagementMode = readFeature(element, u"sm", ns_stream_management);
    d->csiMode = readFeature(element, u"csi", ns_csi);
    d->registerMode = readFeature(element, u"register", ns_register_feature);
    d->preApprovedSubscriptionsSupported = !firstChildElement(element, u"sub", ns_pre_approval).isNull();
    d->rosterVersioningSupported = !firstChildElement(element, u"ver", ns_rosterver).isNull();

    // parse advertised compression methods
    auto compression = firstChildElement(element, u"compression", ns_compressFeature);
    for (const auto &subElement : iterChildElements(compression, u"method")) {
        d->compressionMethods << subElement.text();
    }

    // parse advertised SASL Authentication mechanisms
    auto mechs = firstChildElement(element, u"mechanisms", ns_sasl);
    for (const auto &subElement : iterChildElements(mechs, u"mechanism")) {
        d->authMechanisms << subElement.text();
    }

    d->sasl2Feature = Sasl2::StreamFeature::fromDom(firstChildElement(element, u"authentication", ns_sasl_2));
}

static void writeFeature(QXmlStreamWriter *writer, QStringView tagName, QStringView tagNs, QXmppStreamFeatures::Mode mode)
{
    if (mode != QXmppStreamFeatures::Disabled) {
        writer->writeStartElement(toString65(tagName));
        writer->writeDefaultNamespace(toString65(tagNs));
        if (mode == QXmppStreamFeatures::Required) {
            writer->writeEmptyElement(u"required"_s);
        }
        writer->writeEndElement();
    }
}

static void writeBoolenFeature(QXmlStreamWriter *writer, QStringView tagName, QStringView xmlns, bool enabled)
{
    if (enabled) {
        writer->writeStartElement(toString65(tagName));
        writer->writeDefaultNamespace(toString65(xmlns));
        writer->writeEndElement();
    }
}

void QXmppStreamFeatures::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("stream:features"));
    writeFeature(writer, u"bind", ns_bind, d->bindMode);
    writeFeature(writer, u"session", ns_session, d->sessionMode);
    writeFeature(writer, u"auth", ns_authFeature, d->nonSaslAuthMode);
    writeFeature(writer, u"starttls", ns_tls, d->tlsMode);
    writeFeature(writer, u"sm", ns_stream_management, d->streamManagementMode);
    writeFeature(writer, u"csi", ns_csi, d->csiMode);
    writeFeature(writer, u"register", ns_register_feature, d->registerMode);
    writeBoolenFeature(writer, u"sub", ns_pre_approval, d->preApprovedSubscriptionsSupported);
    writeBoolenFeature(writer, u"ver", ns_rosterver, d->rosterVersioningSupported);

    if (!d->compressionMethods.isEmpty()) {
        writer->writeStartElement(QSL65("compression"));
        writer->writeDefaultNamespace(toString65(ns_compressFeature));
        for (const auto &method : std::as_const(d->compressionMethods)) {
            writer->writeTextElement(QSL65("method"), method);
        }
        writer->writeEndElement();
    }
    if (!d->authMechanisms.isEmpty()) {
        writer->writeStartElement(QSL65("mechanisms"));
        writer->writeDefaultNamespace(toString65(ns_sasl));
        for (const auto &mechanism : std::as_const(d->authMechanisms)) {
            writer->writeTextElement(QSL65("mechanism"), mechanism);
        }
        writer->writeEndElement();
    }

    if (d->sasl2Feature) {
        d->sasl2Feature->toXml(writer);
    }

    writer->writeEndElement();
}
/// \endcond
