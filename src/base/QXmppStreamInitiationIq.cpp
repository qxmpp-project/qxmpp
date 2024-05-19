// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppStreamInitiationIq_p.h"
#include "QXmppUtils_p.h"

#include "StringLiterals.h"

#include <QDomElement>

using namespace QXmpp::Private;

/// \cond
QXmppDataForm QXmppStreamInitiationIq::featureForm() const
{
    return m_featureForm;
}

void QXmppStreamInitiationIq::setFeatureForm(const QXmppDataForm &form)
{
    m_featureForm = form;
}

QXmppTransferFileInfo QXmppStreamInitiationIq::fileInfo() const
{
    return m_fileInfo;
}

void QXmppStreamInitiationIq::setFileInfo(const QXmppTransferFileInfo &fileInfo)
{
    m_fileInfo = fileInfo;
}

QString QXmppStreamInitiationIq::mimeType() const
{
    return m_mimeType;
}

void QXmppStreamInitiationIq::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

QXmppStreamInitiationIq::Profile QXmppStreamInitiationIq::profile() const
{
    return m_profile;
}

void QXmppStreamInitiationIq::setProfile(QXmppStreamInitiationIq::Profile profile)
{
    m_profile = profile;
}

QString QXmppStreamInitiationIq::siId() const
{
    return m_siId;
}

void QXmppStreamInitiationIq::setSiId(const QString &id)
{
    m_siId = id;
}

bool QXmppStreamInitiationIq::isStreamInitiationIq(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement(u"si"_s);
    return (siElement.namespaceURI() == ns_stream_initiation);
}

void QXmppStreamInitiationIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement(u"si"_s);
    m_siId = siElement.attribute(u"id"_s);
    m_mimeType = siElement.attribute(u"mime-type"_s);
    if (siElement.attribute(u"profile"_s) == ns_stream_initiation_file_transfer) {
        m_profile = FileTransfer;
    } else {
        m_profile = None;
    }

    for (const auto &itemElement : iterChildElements(siElement)) {
        if (itemElement.tagName() == u"feature" && itemElement.namespaceURI() == ns_feature_negotiation) {
            m_featureForm.parse(itemElement.firstChildElement());
        } else if (itemElement.tagName() == u"file" && itemElement.namespaceURI() == ns_stream_initiation_file_transfer) {
            m_fileInfo.parse(itemElement);
        }
    }
}

void QXmppStreamInitiationIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QSL65("si"));
    writer->writeDefaultNamespace(toString65(ns_stream_initiation));
    writeOptionalXmlAttribute(writer, u"id", m_siId);
    writeOptionalXmlAttribute(writer, u"mime-type", m_mimeType);
    if (m_profile == FileTransfer) {
        writeOptionalXmlAttribute(writer, u"profile", ns_stream_initiation_file_transfer);
    }
    if (!m_fileInfo.isNull()) {
        m_fileInfo.toXml(writer);
    }
    if (!m_featureForm.isNull()) {
        writer->writeStartElement(QSL65("feature"));
        writer->writeDefaultNamespace(toString65(ns_feature_negotiation));
        m_featureForm.toXml(writer);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
