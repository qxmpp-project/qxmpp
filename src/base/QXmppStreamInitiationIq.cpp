// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppConstants_p.h"
#include "QXmppStreamInitiationIq_p.h"
#include "QXmppUtils.h"

#include <QDomElement>

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
    QDomElement siElement = element.firstChildElement(QStringLiteral("si"));
    return (siElement.namespaceURI() == ns_stream_initiation);
}

void QXmppStreamInitiationIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement(QStringLiteral("si"));
    m_siId = siElement.attribute(QStringLiteral("id"));
    m_mimeType = siElement.attribute(QStringLiteral("mime-type"));
    if (siElement.attribute(QStringLiteral("profile")) == ns_stream_initiation_file_transfer) {
        m_profile = FileTransfer;
    } else {
        m_profile = None;
    }

    QDomElement itemElement = siElement.firstChildElement();
    while (!itemElement.isNull()) {
        if (itemElement.tagName() == QStringLiteral("feature") && itemElement.namespaceURI() == ns_feature_negotiation) {
            m_featureForm.parse(itemElement.firstChildElement());
        } else if (itemElement.tagName() == QStringLiteral("file") && itemElement.namespaceURI() == ns_stream_initiation_file_transfer) {
            m_fileInfo.parse(itemElement);
        }
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppStreamInitiationIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("si"));
    writer->writeDefaultNamespace(ns_stream_initiation);
    helperToXmlAddAttribute(writer, QStringLiteral("id"), m_siId);
    helperToXmlAddAttribute(writer, QStringLiteral("mime-type"), m_mimeType);
    if (m_profile == FileTransfer) {
        helperToXmlAddAttribute(writer, QStringLiteral("profile"), ns_stream_initiation_file_transfer);
    }
    if (!m_fileInfo.isNull()) {
        m_fileInfo.toXml(writer);
    }
    if (!m_featureForm.isNull()) {
        writer->writeStartElement(QStringLiteral("feature"));
        writer->writeDefaultNamespace(ns_feature_negotiation);
        m_featureForm.toXml(writer);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
