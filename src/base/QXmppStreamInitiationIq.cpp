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
#include "QXmppStreamInitiationIq_p.h"
#include "QXmppUtils.h"

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

/// \cond
bool QXmppStreamInitiationIq::isStreamInitiationIq(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement("si");
    return (siElement.namespaceURI() == ns_stream_initiation);
}

void QXmppStreamInitiationIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement siElement = element.firstChildElement("si");
    m_siId = siElement.attribute("id");
    m_mimeType = siElement.attribute("mime-type");
    if (siElement.attribute("profile") == ns_stream_initiation_file_transfer)
        m_profile = FileTransfer;
    else
        m_profile = None;

    QDomElement itemElement = siElement.firstChildElement();
    while (!itemElement.isNull())
    {
        if (itemElement.tagName() == "feature" && itemElement.namespaceURI() == ns_feature_negotiation) {
            m_featureForm.parse(itemElement.firstChildElement());
        } else if (itemElement.tagName() == "file" && itemElement.namespaceURI() == ns_stream_initiation_file_transfer) {
            m_fileInfo.parse(itemElement);
        }
        itemElement = itemElement.nextSiblingElement();
    }
}

void QXmppStreamInitiationIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("si");
    writer->writeAttribute("xmlns", ns_stream_initiation);
    helperToXmlAddAttribute(writer, "id", m_siId);
    helperToXmlAddAttribute(writer, "mime-type", m_mimeType);
    if (m_profile == FileTransfer)
        helperToXmlAddAttribute(writer, "profile", ns_stream_initiation_file_transfer);
    if (!m_fileInfo.isNull())
        m_fileInfo.toXml(writer);
    if (!m_featureForm.isNull()) {
        writer->writeStartElement("feature");
        writer->writeAttribute("xmlns", ns_feature_negotiation);
        m_featureForm.toXml(writer);
        writer->writeEndElement();
    }
    writer->writeEndElement();
}
/// \endcond
